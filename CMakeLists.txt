cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(
  statistical-simulation
  VERSION 0.0.1
)

# Add libraries from external sources.
add_subdirectory(external)

# A library for reading/writing Protobuf files.
add_subdirectory(ioproto)

# A library for reading/writing gem5 traces.
add_subdirectory(iogem5)

# Small helper executables.
add_subdirectory(util)
