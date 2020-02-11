project(
  mocktails-trace-generator
  VERSION 0.0.1
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
    statistical-simulation::mocktails-model
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
