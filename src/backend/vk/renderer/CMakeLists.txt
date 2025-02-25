
find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(SHADERC REQUIRED shaderc)

make_shaders(shader.cc.in shader.cc)

add_library(vk_renderer SHARED
        error.h
        handle.h
        object.cc
        object.h
        object_loader.h
        object_loader.cc
        commander.h
        commander.cc
        instance.h
        instance.cc
        physical_device.h
        physical_device.cc
        device.h
        device.cc
        device_selector.h
        device_selector.cc
        commander.h
        commander.cc
        plugin.cc
        window.h
        renderer.cc
        renderer.h
        buffer.h
        image.h
        memory.h
        swapchain.cc
        swapchain.h
        shader.h
        shader.cc
)

set_property(TARGET vk_renderer PROPERTY POSITION_INDEPENDENT_CODE ON)

target_compile_definitions(vk_renderer PRIVATE -DENGINE_SHARED -DENGINE_EXPORT -DGLM_FORCE_RADIANS -DGLM_FORCE_DEPTH_ZERO_TO_ONE)
target_link_directories(vk_renderer PUBLIC ${SHADERC_LIBRARY_DIRS})
target_link_libraries(vk_renderer PUBLIC
        Vulkan::Vulkan
        ${SHADERC_LIBRARIES}
        obj
)