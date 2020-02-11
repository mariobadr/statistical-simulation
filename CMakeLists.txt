cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(
  statistical-simulation
  VERSION 0.0.1
)

# Globally set that all executables be compiled into a 'bin' directory.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Warning flags to use for different compilers.
set(STATSIM_MSVC_WARNING_FLAGS /W4 /WX)
set(STATSIM_GCC_WARNING_FLAGS -Wall -Wextra -Wnon-virtual-dtor -pedantic -Wold-style-cast -Wcast-align
  -Woverloaded-virtual -Wconversion -Wsign-conversion)

# Add libraries from external sources.
add_subdirectory(external)

# A library for reading/writing Protobuf files.
add_subdirectory(ioproto)
# A library for reading/writing gem5 traces.
add_subdirectory(iogem5)

# A library for calculating the reuse distance between memory requests.
add_subdirectory(reuse-distance)

# The project for creating STM models and traces.
add_subdirectory(stm)
# The project for creating HRD models and traces.
add_subdirectory(hrd)
# The project for creating Mocktails models and traces.
add_subdirectory(mocktails)

# Small helper executables.
add_subdirectory(util)
