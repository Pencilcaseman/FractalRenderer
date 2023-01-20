cmake_minimum_required(VERSION 3.10)
project(FractalRenderer)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE FRACTAL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(FractalRenderer main.cpp ${FRACTAL_SOURCES})

add_subdirectory(cinderbox)
add_subdirectory(glsl-optimizer)
target_link_libraries(FractalRenderer PUBLIC cinderbox glsl_optimizer)

target_include_directories(FractalRenderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(FractalRenderer PUBLIC -DFRACTAL_RENDERER_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
