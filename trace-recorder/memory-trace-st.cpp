#include "pin.H"

#include <fstream>
#include <iostream>

KNOB<INT64> KnobMaxMemoryRequests(KNOB_MODE_WRITEONCE, "pintool", "m", "1000000000", "max memory requests");
KNOB<INT64> KnobMaxInstructions(KNOB_MODE_WRITEONCE, "pintool", "i", "250000000", "max instructions");
KNOB<INT64> KnobSkipInstructions(KNOB_MODE_WRITEONCE, "pintool", "f", "10000000000", "fast forward this many instructions");
KNOB<std::string> KnobOutput(KNOB_MODE_WRITEONCE, "pintool", "o", "trace.csv", "trace file name");

FILE * G_Trace;
NATIVE_FD G_PageMap = 0;
USIZE G_PageSize = 0;

INT64 G_SkipInstructionCount = 0;
INT64 G_MaxInstructionCount = 0;
INT64 G_InstructionCount = 0;
INT64 G_MaxMemoryRequests = 0;
INT64 G_MemoryRequestCount = 0;

UINT64 GetPageFrameNumber(ADDRINT virtualAddress)
{
  OS_RETURN_CODE code;

  ADDRINT const virtualPageNumber = virtualAddress / G_PageSize;
  USIZE size = sizeof(UINT64);
  INT64 offset = virtualPageNumber * size;

  code = OS_SeekFD(G_PageMap, OS_FILE_SEEK_SET, &offset);
  if(code.generic_err == OS_RETURN_CODE_FILE_SEEK_FAILED) {
    PIN_WriteErrorMessage("Could not seek to offset %ld.\n", 1000, PIN_ERR_FATAL, 1, offset);
  }

  UINT64 data = 0;
  code = OS_ReadFD(G_PageMap, &size, &data);
  if(code.generic_err == OS_RETURN_CODE_FILE_READ_FAILED) {
    PIN_WriteErrorMessage("Could not read from pagemap file.\n", 1000, PIN_ERR_FATAL, 0);
  }

  return data & (((uint64_t)1 << 54) - 1);
}

ADDRINT VirtualToPhysical(ADDRINT virtualAddress)
{
  UINT64 pageFrameNumber = GetPageFrameNumber(virtualAddress);

  return ((pageFrameNumber * G_PageSize) + (virtualAddress % G_PageSize));
}

VOID DumpToTrace(INT32 command, ADDRINT address, INT32 size, ADDRINT pc)
{
  address = VirtualToPhysical(address);
  pc = VirtualToPhysical(pc);

  fprintf(G_Trace, "%ld,%d,%lu,%d,%lu\n", G_InstructionCount, command, address, size, pc);
}

VOID Read(VOID * pc, VOID * address, INT32 size, BOOL is_prefetch)
{
  if(G_InstructionCount < G_SkipInstructionCount) {
    return;
  }

  INT64 const instructionCount = G_InstructionCount - G_SkipInstructionCount;
  if(G_MemoryRequestCount < G_MaxMemoryRequests && instructionCount < G_MaxInstructionCount) {
    DumpToTrace(0, (ADDRINT) address, size, (ADDRINT) pc);

    G_MemoryRequestCount++;
    return;
  }

  PIN_ExitApplication(0);
}

VOID *G_WriteAddress = NULL;
INT32 G_WriteSize = 0;

VOID SaveWriteInfo(VOID * address, INT32 size)
{
  // save the address and size in global state.
  G_WriteAddress = address;
  G_WriteSize = size;
}

VOID Write(VOID * pc)
{
  if(G_InstructionCount < G_SkipInstructionCount) {
    return;
  }

  INT64 const instructionCount = G_InstructionCount - G_SkipInstructionCount;
  if(G_MemoryRequestCount < G_MaxMemoryRequests && instructionCount < G_MaxInstructionCount) {
    DumpToTrace(1, (ADDRINT) G_WriteAddress, G_WriteSize, (ADDRINT) pc);

    G_MemoryRequestCount++;
    return;
  }

  PIN_ExitApplication(0);
}

VOID PIN_FAST_ANALYSIS_CALL IncrementInstructionCount()
{
  ++G_InstructionCount;
}

VOID Instruction(INS ins, VOID *)
{
  // Track the number of instructions executed.
  INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IncrementInstructionCount, IARG_END);

  // Only instrument loads/stores that are actually executed.
  // This is based on source/tools/SimpleExamples/pinatrace.cpp,

  if (INS_IsMemoryRead(ins) && INS_IsStandardMemop(ins)) {
    INS_InsertPredicatedCall(
      ins, IPOINT_BEFORE, (AFUNPTR)Read,
      // track the PC
      IARG_INST_PTR,
      // track the memory address being read
      IARG_MEMORYREAD_EA,
      // track the size of the load
      IARG_MEMORYREAD_SIZE,
      // track whether or not this is a prefetch
      IARG_BOOL, INS_IsPrefetch(ins),
      IARG_END);
  }

  // for instructions with 2 memory read operands
  if (INS_HasMemoryRead2(ins) && INS_IsStandardMemop(ins)) {
    INS_InsertPredicatedCall(
      ins, IPOINT_BEFORE, (AFUNPTR)Read,
      // track the PC
      IARG_INST_PTR,
      // track the memory address being read
      IARG_MEMORYREAD2_EA,
      // track the size of the load
      IARG_MEMORYREAD_SIZE,
      // track whether or not this is a prefetch
      IARG_BOOL, INS_IsPrefetch(ins),
      IARG_END);
  }

  // instruments stores using a predicated call, i.e.
  // the call happens iff the store will be actually executed
  if (INS_IsMemoryWrite(ins) && INS_IsStandardMemop(ins)) {
    // need to save this information at IPOINT_BEFORE
    INS_InsertPredicatedCall(
      ins, IPOINT_BEFORE, (AFUNPTR)SaveWriteInfo,
      IARG_MEMORYWRITE_EA,
      IARG_MEMORYWRITE_SIZE,
      IARG_END);

    if (INS_HasFallThrough(ins)) {
      // the next (i.e., immediately after) instruction will be executed
      INS_InsertCall(
        ins, IPOINT_AFTER, (AFUNPTR)Write,
        IARG_INST_PTR,
        IARG_END);
    }

    if (INS_IsBranchOrCall(ins)) {
      // not entirely sure why this is here...
      // can a write instruction also be a branch or a call?
      INS_InsertCall(
        ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)Write,
        IARG_INST_PTR,
        IARG_END);
    }

  }
}

VOID Fini(INT32 code, VOID *v)
{
  OS_CloseFD(G_PageMap);
  fclose(G_Trace);

  fprintf(stdout, "Instrumented %ld memory references from %ld to %ld instructions.\n", G_MemoryRequestCount,
    G_SkipInstructionCount, G_InstructionCount);
}

INT32 LoadPageMap()
{
  // Based on: https://stackoverflow.com/a/45128487
  char pagemapFilename[BUFSIZ];
  snprintf(pagemapFilename, sizeof(pagemapFilename), "/proc/%ju/pagemap", (uintmax_t) PIN_GetPid());

  OS_RETURN_CODE code;

  code = OS_OpenFD(pagemapFilename, OS_FILE_OPEN_TYPE_READ, OS_FILE_PERMISSION_TYPE_READ, &G_PageMap);
  if (code.generic_err == OS_RETURN_CODE_FILE_OPEN_FAILED) {
    return EXIT_FAILURE;
  } else {
    fprintf(stdout, "Process %d's pagemap was successfully opened.\n", PIN_GetPid());
  }

  code = OS_GetPageSize(&G_PageSize);
  if (code.generic_err == OS_RETURN_CODE_QUERY_FAILED) {
    return EXIT_FAILURE;
  } else {
    fprintf(stdout, "The page size is %lu bytes.\n", G_PageSize);
  }

  fflush(stdout);
  return EXIT_SUCCESS;
}

INT32 Usage()
{
  std::cerr << KNOB_BASE::StringKnobSummary() << "\n";
  return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
  PIN_InitSymbols();
  if(PIN_Init(argc,argv)) {
    return Usage();
  }

  G_SkipInstructionCount = KnobSkipInstructions.Value();
  G_MaxMemoryRequests = KnobMaxMemoryRequests.Value();
  G_MaxInstructionCount = KnobMaxInstructions.Value();
  G_Trace = fopen(KnobOutput.Value().c_str(), "w");

  INS_AddInstrumentFunction(Instruction, 0);
  PIN_AddFiniFunction(Fini, NULL);

  if(LoadPageMap() == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  // never returns
  PIN_StartProgram();

  return 0;
}
