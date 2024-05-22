cmake_minimum_required(VERSION 3.22)
project(untitled70)

set(CMAKE_CXX_STANDARD 17)


find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

include_directories("src")

add_executable(untitled70 src/main.cpp
        src/vk/instance.h
        src/base/exception.h
        src/vk/instance.cc
        src/vk/debug.cc
        src/vk/common.h
        src/vk/common.cc
        src/glfw/window.h
        src/glfw/window.cc
        src/glfw/exception.h
        src/engine.h
        src/engine.cc
        src/vk/device.h
        src/vk/device.cc
        src/vk/surface.h
        src/vk/surface.cc)

target_link_libraries(untitled70 Vulkan::Vulkan glfw)

#add_compile_definitions(DEBUG)
