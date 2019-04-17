project(
  truncate-gem5-trace
  VERSION 0.0.1
  LANGUAGES CXX
)

add_executable(
  ${PROJECT_NAME}
  src/main.cpp
)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
    argagg::argagg
    statistical-simulation::iogem5
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
