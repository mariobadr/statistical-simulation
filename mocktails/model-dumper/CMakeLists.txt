project(
  dump-mocktails-profile
  VERSION 0.0.1
  LANGUAGES CXX
)

add_executable(
  ${PROJECT_NAME}
  src/main.cpp
)

target_link_libraries(
  dump-mocktails-profile
  PRIVATE
    argagg::argagg
    spdlog::spdlog
    statistical-simulation::mocktails-model
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
