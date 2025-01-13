
find_package(SDL2 REQUIRED)

set(SOURCES
        error.h
        instance.cc
        instance.h
        window.cc
        window.h
)

add_library(sdl_internal STATIC ${SOURCES})

target_include_directories(sdl_internal PUBLIC ${SDL2_INCLUDE_DIRS})

target_link_libraries(sdl_internal PUBLIC ${SDL2_LIBRARIES})