find_package(Protobuf REQUIRED)

project(
  mocktails-model
  VERSION 0.0.1
  LANGUAGES CXX
)

protobuf_generate_cpp(
  MOCKTAILS_PROTO_PROFILE_SOURCES
  MOCKTAILS_PROTO_PROFILE_HEADERS
  proto/mocktails.proto
)

add_library(
  ${PROJECT_NAME}
  ${MOCKTAILS_PROTO_PROFILE_SOURCES}
  ${MOCKTAILS_PROTO_PROFILE_HEADERS}
  include/mocktails/address.hpp
  include/mocktails/hierarchy.hpp
  include/mocktails/metadata.hpp
  include/mocktails/model.hpp
  include/mocktails/partition.hpp
  include/mocktails/profile.hpp
  include/mocktails/request.hpp
  include/mocktails/synthesis.hpp
  src/address.cpp
  src/hierarchy.cpp
  src/metadata.cpp
  src/model.cpp
  src/partition.cpp
  src/profile.cpp
  src/request.cpp
  src/synthesis.cpp
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
    statistical-simulation::reuse-distance
    statistical-simulation::hrd-model
    statistical-simulation::stm-model
)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
)

if(MSVC)
  target_compile_options(
    ${PROJECT_NAME}
    PRIVATE
      ${STATSIM_MSVC_WARNING_FLAGS}
  )
else()
  target_compile_options(
    ${PROJECT_NAME}
    PRIVATE
      ${STATSIM_GCC_WARNING_FLAGS}
  )
endif()

