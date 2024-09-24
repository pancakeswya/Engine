cmake_minimum_required(VERSION 3.22)
project(vk_triangle)

set(CMAKE_CXX_STANDARD 17)

set(SRC_DIR src)
set(EXTERNAL third_party)

add_subdirectory(shaders)
add_compile_definitions(DEBUG)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

include_directories("${SRC_DIR}")
include_directories("${EXTERNAL}/glm")

add_executable(vk_triangle
        ${SRC_DIR}/main.cc
        ${SRC_DIR}/app/app.cc
        ${SRC_DIR}/app/app.h
        ${SRC_DIR}/base/io.cc
        ${SRC_DIR}/base/io.h
        ${SRC_DIR}/backend/render/vk_backend.cc
        ${SRC_DIR}/backend/render/vk_backend.h
        ${SRC_DIR}/backend/render/vk_config.cc
        ${SRC_DIR}/backend/render/vk_config.h
        ${SRC_DIR}/backend/render/vk_factory.cc
        ${SRC_DIR}/backend/render/vk_factory.h
        ${SRC_DIR}/backend/window/glfw.h
        ${SRC_DIR}/backend/window/glfw.cc
)

target_link_libraries(vk_triangle Vulkan::Vulkan glfw)