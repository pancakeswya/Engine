cmake_minimum_required(VERSION 3.22)
project(untitled7 C)

set(CMAKE_C_STANDARD 11)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_subdirectory(shaders)

include_directories("src")

add_executable(untitled7
        src/main.c
        src/base/error.h
        src/base/io.h
        src/base/io.c
        src/vk/context.h
        src/vk/context.c
        src/vk/swapchain.h
        src/vk/swapchain.c
        src/vk/device.h
        src/vk/device.c
        src/vk/sync.h
        src/vk/sync.c)

target_link_libraries(untitled7 Vulkan::Vulkan glfw)

add_compile_definitions(DEBUG)