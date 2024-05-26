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
        src/vk/layers.h
        src/vk/layers.cc
        src/vk/swap_chain.cc
        src/vk/pipeline.h
        src/base/io.h
        src/base/io.cc
        src/vk/pipeline.cc
        src/vk/shader.h
        src/vk/shader.cc
        src/vk/render_pass.h
        src/vk/render_pass.cc
        src/vk/command.h
        src/vk/command.cc
        src/vk/sync.h
        src/vk/sync.cc
        src/vk/framebuffer.h
        src/vk/framebuffer.cc
        src/vk/context.h
        src/vk/context.cc
        src/render/render.h
        src/render/render.cc
)

target_link_libraries(untitled70 Vulkan::Vulkan glfw)

add_compile_definitions(DEBUG)
