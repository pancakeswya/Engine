
pkg_check_modules(SHADERC REQUIRED shaderc)

set(SHADERS
        shaders/simple.vert
        shaders/simple.frag
)

foreach (SHADER ${SHADERS})
    get_filename_component(SHADERNAME ${SHADER} LAST_EXT)
    string(SUBSTRING "${SHADERNAME}" 1 -1 SHADERNAME)
    string(TOUPPER ${SHADERNAME} SHADERNAME)
    file(READ ${SHADER} ${SHADERNAME})
endforeach ()

configure_file(shaders.cc.in shaders.cc @ONLY)

add_library(vk_renderer STATIC
        config.cc
        config.h
        renderer.cc
        renderer.h
        error.h
        dispatchable.h
        object.h
        object_loader.h
        object_loader.cc
        commander.h
        commander.cc
        instance.h
        instance.cc
        device.h
        device.cc
        device_selector.h
        device_selector.cc
        commander.h
        commander.cc
        shaders.h
        shaders.cc
)

target_link_directories(vk_renderer PUBLIC ${SHADERC_LIBRARY_DIRS})

target_link_libraries(vk_renderer
        Vulkan::Vulkan
        ${SHADERC_LIBRARIES}
        obj
        renderer
)

add_library(vk_renderer-entry SHARED
        entry.cc
)

target_link_libraries(vk_renderer-entry PUBLIC vk_renderer)