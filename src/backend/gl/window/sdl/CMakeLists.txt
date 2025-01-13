
set(SOURCES
        error.h
        instance.h
        window.h
)

add_library(gl_sdl_window SHARED ${SOURCES})

target_link_libraries(gl_sdl_window PUBLIC sdl_internal)