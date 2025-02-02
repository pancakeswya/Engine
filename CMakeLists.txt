cmake_minimum_required(VERSION 3.22)
project(engine)

set(CMAKE_CXX_STANDARD 17)

#add_compile_definitions(DEBUG)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(cmake/shaders.cmake)

include_directories(src)
include_directories(third_party/glm)
include_directories(third_party/stb)
include_directories(third_party/earcut.hpp/include)

add_subdirectory(src)

