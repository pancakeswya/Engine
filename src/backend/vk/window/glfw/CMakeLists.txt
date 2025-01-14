
set(SOURCES
        error.h
        instance.h
        window.h
        window.cc
)

find_package(glfw3 REQUIRED)

add_library(glfw_vk_window STATIC ${SOURCES})

target_link_libraries(glfw_vk_window PUBLIC Vulkan::Vulkan glfw_internal)

add_library(glfw_vk_window-plugin SHARED plugin.cc)

target_link_libraries(glfw_vk_window-plugin PUBLIC glfw_vk_window)