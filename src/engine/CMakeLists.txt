
set(SOURCES
        render/data_util.h
        render/model.h
        render/renderer.cc
        render/renderer.h
        render/plugin.h
        render/types.h

        window/instance.h
        window/window.cc
        window/window.h
        window/plugin.h

        dll_loader.h
)

add_library(renderer STATIC ${SOURCES})