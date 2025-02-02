
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_library(glfw_vk_window SHARED
        error.h
        plugin.cc
        instance.h
        window.h
        window.cc
)

set_property(TARGET glfw_vk_window PROPERTY POSITION_INDEPENDENT_CODE ON)

target_compile_definitions(glfw_vk_window PRIVATE -DENGINE_SHARED -DENGINE_EXPORT)
target_link_libraries(glfw_vk_window PUBLIC Vulkan::Vulkan glfw)