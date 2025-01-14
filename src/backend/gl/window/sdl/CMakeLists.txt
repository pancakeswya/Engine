
set(SOURCES
        error.h
        instance.h
        window.cc
        window.h
)

add_library(sdl_gl_window STATIC ${SOURCES})

target_link_libraries(sdl_gl_window PUBLIC sdl_internal)

add_library(sdl_gl_window-plugin SHARED plugin.cc)

target_link_libraries(sdl_gl_window-plugin PUBLIC sdl_gl_window)