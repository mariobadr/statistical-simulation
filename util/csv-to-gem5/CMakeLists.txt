project(
  convert-to-gem5
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
