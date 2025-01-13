cmake_minimum_required(VERSION 3.22)
project(engine)

set(CMAKE_CXX_STANDARD 17)

set(SRC_DIR src)
set(EXTERNAL third_party)

# add_compile_definitions(DEBUG)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PkgConfig REQUIRED)

include_directories("${SRC_DIR}")
include_directories("${EXTERNAL}/glm")
include_directories("${EXTERNAL}/stb")
include_directories("${EXTERNAL}/earcut.hpp/include")

add_subdirectory(${SRC_DIR}/engine)
add_subdirectory(${SRC_DIR}/obj)

add_subdirectory(${SRC_DIR}/backend)

add_executable(engine
        ${SRC_DIR}/main.cc
        ${SRC_DIR}/app/app.cc
        ${SRC_DIR}/app/app.h
)

target_link_libraries(engine renderer)