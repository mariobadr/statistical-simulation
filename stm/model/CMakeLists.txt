find_package(Protobuf REQUIRED)

project(
  stm-model
  VERSION 0.0.1
  LANGUAGES CXX
)

protobuf_generate_cpp(
  PROTO_STM_PROFILE_SOURCES
  PROTO_STM_PROFILE_HEADERS
  proto/stm.proto
)

add_library(
  ${PROJECT_NAME}
  ${PROTO_STM_PROFILE_SOURCES}
  ${PROTO_STM_PROFILE_HEADERS}
  include/stm/metadata.hpp
  include/stm/profile.hpp
  include/stm/stack-distance.hpp
  include/stm/stride-pattern.hpp
  include/stm/synthesis.hpp
  src/hash.hpp
  src/metadata.cpp
  src/profile.cpp
  src/stack-distance.cpp
  src/stride-pattern.cpp
  src/synthesis.cpp
)

add_library(statistical-simulation::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC
    statistical-simulation::ioproto
    statistical-simulation::reuse-distance
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
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
