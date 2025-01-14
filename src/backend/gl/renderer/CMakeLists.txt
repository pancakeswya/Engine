
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

add_library(gl_renderer SHARED
        error.h
        error.cc
        plugin.cc
        renderer.cc
        renderer.h
        window.h
)

target_link_libraries(gl_renderer PUBLIC
        obj
        OpenGL::GL
        GLEW::GLEW
)