
find_package(glfw3 REQUIRED)

add_library(glfw_gl_window SHARED
        error.h
        instance.h
        plugin.cc
        window.cc
        window.h
)

target_link_libraries(glfw_gl_window PUBLIC glfw)