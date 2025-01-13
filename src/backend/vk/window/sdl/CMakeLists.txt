
set(SOURCES
        error.h
        instance.h
        instance.cc
        window.h
        window.cc
)

add_library(sdl_vk_window STATIC ${SOURCES})

target_link_libraries(sdl_vk_window PUBLIC Vulkan::Vulkan sdl_internal)

add_library(sdl_vk_window-entry SHARED entry.cc)

target_link_libraries(sdl_vk_window-entry PUBLIC sdl_vk_window)