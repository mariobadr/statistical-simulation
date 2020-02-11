project(
  mocktails-model-generator
  VERSION 0.0.1
  LANGUAGES CXX
)

add_executable(
  ${PROJECT_NAME}
  src/config.cpp
  src/config.hpp
  src/json.hpp
  src/main.cpp
  src/modelgen.cpp
  src/modelgen.hpp
)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
    argagg::argagg
    spdlog::spdlog
    statistical-simulation::iogem5
    statistical-simulation::mocktails-model
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
