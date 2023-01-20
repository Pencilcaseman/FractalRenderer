cmake_minimum_required(VERSION 3.10)
project(FractalRenderer)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE FRACTAL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(FractalRenderer main.cpp ${FRACTAL_SOURCES})

# Include multiprecision floating point
set(LIBRAPID_USE_MULTIPREC ON)

add_subdirectory(cinderbox)
add_subdirectory(json)
target_link_libraries(FractalRenderer PUBLIC cinderbox nlohmann_json::nlohmann_json)

target_include_directories(FractalRenderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(FractalRenderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thread-pool)

target_compile_definitions(FractalRenderer PUBLIC -DFRACTAL_RENDERER_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
