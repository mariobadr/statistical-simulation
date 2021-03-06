project(
  stm-trace-generator
  VERSION 1.0.0
  LANGUAGES CXX
)

add_executable(
  ${PROJECT_NAME}
  src/main.cpp
  src/tracegen.cpp
  src/tracegen.hpp
)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
    argagg::argagg
    spdlog::spdlog
    statistical-simulation::iogem5
    statistical-simulation::stm-model
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
