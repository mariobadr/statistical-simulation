# Authors: Mario Badr

from m5.params import *
from m5.proxy import *
from MemObject import MemObject

class PacketTraceGen(MemObject):
        type = 'PacketTraceGen'
        cxx_header = "cpu/testers/packet_trace/packet_trace_gen.hh"

        # File to parse that describes the statistical profile (model)
        trace = Param.String("Trace file for this generator.")

        outstanding = Param.Int("Maximum outstanding requests.")
        max_packets = Param.Int("Maximum packets to generate.")
        cache_align = Param.Bool("Align packets to cache lines.")

        # System used to determine the mode of the memory system
        system = Param.System(Parent.any, "System this device is part of")

        # Port used for sending requests and receiving responses
        port = MasterPort("Master port")
