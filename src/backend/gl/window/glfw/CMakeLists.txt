
set(SOURCES
        error.h
        instance.h
        window.cc
        window.h
)

add_library(glfw_gl_window STATIC ${SOURCES})

target_link_libraries(glfw_gl_window PUBLIC glfw_internal)

add_library(glfw_gl_window-plugin SHARED plugin.cc)

target_link_libraries(glfw_gl_window-plugin PUBLIC glfw_gl_window)