
find_package(SDL2 REQUIRED)

add_library(sdl_gl_window SHARED
        error.h
        instance.h
        plugin.cc
        window.cc
        window.h
)

set_property(TARGET sdl_gl_window PROPERTY POSITION_INDEPENDENT_CODE ON)

target_compile_definitions(sdl_gl_window PRIVATE -DENGINE_SHARED -DENGINE_EXPORT)
target_include_directories(sdl_gl_window PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(sdl_gl_window PUBLIC ${SDL2_LIBRARIES})

