
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

set(SOURCES
        error.h
        error.cc
        renderer.cc
        renderer.h
)

add_library(gl_renderer SHARED ${SOURCES})

target_link_libraries(gl_renderer PUBLIC
        obj
        renderer
        OpenGL::GL
        GLEW::GLEW
)