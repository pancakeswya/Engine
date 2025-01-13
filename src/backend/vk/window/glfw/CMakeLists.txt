
set(SOURCES
        error.h
        instance.h
        window.h
        window.cc
)

find_package(glfw3 REQUIRED)

add_library(glfw_vk_window STATIC ${SOURCES})

target_link_libraries(glfw_vk_window PUBLIC Vulkan::Vulkan glfw_internal)

add_library(glfw_vk_window-entry SHARED entry.cc)

target_link_libraries(glfw_vk_window-entry PUBLIC glfw_vk_window)