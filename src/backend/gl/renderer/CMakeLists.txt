
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

make_shaders(shaders.cc.in shaders.cc)

add_library(gl_renderer SHARED
        error.h
        error.cc
        plugin.cc
        object.h
        object_loader.cc
        object_loader.h
        handle_object.cc
        handle_object.h
        renderer.cc
        renderer.h
        shaders.cc
        shaders.h
        window.h
)

target_link_libraries(gl_renderer PUBLIC
        obj
        OpenGL::GL
        GLEW::GLEW
)