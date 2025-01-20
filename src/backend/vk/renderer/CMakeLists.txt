
find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(SHADERC REQUIRED shaderc)

make_shaders(shader.cc.in shader.cc)

add_library(vk_renderer SHARED
        config.cc
        config.h
        error.h
        dispatchable.h
        object.h
        object_loader.h
        object_loader.cc
        commander.h
        commander.cc
        instance.h
        instance.cc
        instance_dispatchable_factory.cc
        instance_dispatchable_factory.h
        device.h
        device.cc
        device_selector.h
        device_selector.cc
        device_dispatchable_factory.cc
        device_dispatchable_factory.h
        commander.h
        commander.cc
        plugin.cc
        window.h
        renderer.cc
        renderer.h
        buffer.cc
        buffer.h
        image.cc
        image.h
        swapchain.cc
        swapchain.h
        shader.h
        shader.cc
)

set_property(TARGET vk_renderer PROPERTY POSITION_INDEPENDENT_CODE ON)

target_compile_definitions(vk_renderer PRIVATE -DGLM_FORCE_RADIANS -DGLM_FORCE_DEPTH_ZERO_TO_ONE)
target_link_directories(vk_renderer PUBLIC ${SHADERC_LIBRARY_DIRS})
target_link_libraries(vk_renderer PUBLIC
        Vulkan::Vulkan
        ${SHADERC_LIBRARIES}
        obj
)