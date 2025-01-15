
find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(SHADERC REQUIRED shaderc)

make_shaders(shaders.cc.in shaders.cc)

add_library(vk_renderer SHARED
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
        plugin.cc
        window.h
        shaders.h
        shaders.cc
)

target_compile_definitions(vk_renderer PRIVATE -DGLM_FORCE_RADIANS -DGLM_FORCE_DEPTH_ZERO_TO_ONE)
target_link_directories(vk_renderer PUBLIC ${SHADERC_LIBRARY_DIRS})
target_link_libraries(vk_renderer
        Vulkan::Vulkan
        ${SHADERC_LIBRARIES}
        obj
)