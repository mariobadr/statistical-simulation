find_package(Protobuf REQUIRED)

project(
  iogem5
  VERSION 0.0.1
  LANGUAGES CXX
)

protobuf_generate_cpp(
  PROTO_GEM5_PACKET_SOURCES
  PROTO_GEM5_PACKET_HEADERS
  proto/packet.proto
)

add_library(
  ${PROJECT_NAME}
  ${PROTO_GEM5_PACKET_SOURCES}
  ${PROTO_GEM5_PACKET_HEADERS}
  include/iogem5/packet-trace.hpp
  src/packet-trace.cpp
)

add_library(statistical-simulation::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC
    statistical-simulation::ioproto
)

if(MSVC)
  target_compile_options(
    ${PROJECT_NAME}
    PRIVATE
      /W4 /WX
  )
else()
  target_compile_options(
    ${PROJECT_NAME}
    PRIVATE
      -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wold-style-cast -Wcast-align -Woverloaded-virtual
      -Wconversion -Wsign-conversion
  )
endif()

