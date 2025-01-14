
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

set(SOURCES
        error.h
        error.cc
        plugin.cc
        renderer.cc
        renderer.h
)

add_library(gl_renderer SHARED ${SOURCES})

target_link_libraries(gl_renderer PUBLIC
        obj
        OpenGL::GL
        GLEW::GLEW
)

add_library(gl_renderer-plugin SHARED plugin.cc)

target_link_libraries(gl_renderer-plugin PUBLIC gl_renderer)