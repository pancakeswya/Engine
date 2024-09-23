cmake_minimum_required(VERSION 3.22)
project(vk_triangle)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(shaders)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

include_directories("src")

add_executable(vk_triangle
        src/main.cc
        src/app/app.cc
        src/app/app.h
        src/base/io.cc
        src/base/io.h
        src/backend/render/vk_backend.cc
        src/backend/render/vk_backend.h
        src/backend/render/vk_config.cc
        src/backend/render/vk_config.h
        src/backend/render/vk_factory.cc
        src/backend/render/vk_factory.h
        src/backend/window/glfw.h
        src/backend/window/glfw.cc
)

target_link_libraries(vk_triangle Vulkan::Vulkan glfw)