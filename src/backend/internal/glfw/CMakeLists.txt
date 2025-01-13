
set(SOURCES
        error.h
        instance.cc
        instance.h
        window.cc
        window.h
)

find_package(glfw3 REQUIRED)

add_compile_definitions(-D_GLFW_VULKAN_LIBRARY="/opt/homebrew/opt/vulkan-loader/lib/libvulkan.1.dylib")
add_library(glfw_internal STATIC ${SOURCES})

target_link_libraries(glfw_internal glfw)
