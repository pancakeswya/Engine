cmake_minimum_required(VERSION 3.22)
project(untitled70)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(shaders)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

include_directories("src")

add_executable(untitled70
        src/main.cpp
        src/engine.h
        src/engine.cc
        src/base/exception.h
        src/vk/instance.h
        src/vk/instance.cc
        src/vk/messenger.cc
        src/vk/messenger.h
        src/vk/devices.h
        src/vk/devices.cc
        src/vk/surface.h
        src/vk/surface.cc
        src/vk/swap_chain.h
        src/glfw/window.h
        src/glfw/window.cc
        src/glfw/exception.h
        src/vk/common.h
        src/vk/common.cc
        src/vk/swap_chain.cc
        src/vk/queue.h
        src/vk/queue.cc
        src/vk/graphics.h
        src/base/io.h
        src/base/io.cc
        src/vk/graphics.cc
        src/vk/shader.h
        src/vk/shader.cc
        src/vk/render.h
        src/vk/render.cc
)

target_link_libraries(untitled70 Vulkan::Vulkan glfw)

# add_compile_definitions(DEBUG)
