
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_library(glfw_vk_window SHARED
        error.h
        plugin.cc
        instance.h
        window.h
        window.cc
)

target_link_libraries(glfw_vk_window PUBLIC Vulkan::Vulkan glfw)