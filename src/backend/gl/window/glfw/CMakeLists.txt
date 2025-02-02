
find_package(glfw3 REQUIRED)

add_library(glfw_gl_window SHARED
        error.h
        instance.h
        plugin.cc
        window.cc
        window.h
)

set_property(TARGET glfw_gl_window PROPERTY POSITION_INDEPENDENT_CODE ON)

target_compile_definitions(glfw_gl_window PRIVATE -DENGINE_SHARED -DENGINE_EXPORT)
target_link_libraries(glfw_gl_window PUBLIC glfw)