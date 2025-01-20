
find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

add_library(sdl_vk_window SHARED
        error.h
        instance.h
        instance.cc
        plugin.cc
        window.h
        window.cc
)

set_property(TARGET sdl_vk_window PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(sdl_vk_window PUBLIC ${SDL2_INCLUDE_DIRS})

target_link_libraries(sdl_vk_window PUBLIC Vulkan::Vulkan ${SDL2_LIBRARIES})