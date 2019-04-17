find_package(Protobuf REQUIRED)

project(
  ioproto
  VERSION 0.0.1
  LANGUAGES CXX
)

add_library(
  ${PROJECT_NAME}
  include/ioproto/istream.hpp
  include/ioproto/ofstream.hpp
  src/istream.cpp
  src/ofstream.cpp
)

add_library(statistical-simulation::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    # Older versions of CMake may not do this if not set explicitly?
    ${PROTOBUF_INCLUDE_DIRS}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC
    # For newer versions of CMake
    ${Protobuf_LIBRARIES}
    # For older versions of CMake
    ${PROTOBUF_LIBRARIES}
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

