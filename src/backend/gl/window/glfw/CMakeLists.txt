
set(SOURCES
        error.h
        instance.h
        window.cc
        window.h
)

add_library(gl_glfw_window SHARED ${SOURCES})

target_link_libraries(gl_glfw_window PUBLIC glfw_internal)