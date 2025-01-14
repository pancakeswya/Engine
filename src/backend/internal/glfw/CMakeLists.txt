
set(SOURCES
        error.h
        instance.cc
        instance.h
        window.cc
        window.h
)

find_package(glfw3 REQUIRED)

add_library(glfw_internal STATIC ${SOURCES})

target_link_libraries(glfw_internal PUBLIC glfw)
