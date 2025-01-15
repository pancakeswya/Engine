
set(SOURCES
        render/data_util.h
        render/model.h
        render/renderer_loader.cc
        render/renderer_loader.h
        render/renderer.h
        render/plugin.h
        render/types.h

        window/instance.h
        window/window_loader.cc
        window/window_loader.h
        window/window.h
        window/plugin.h

        dll_loader.h
        runner.cc
        runner.h
)

add_library(engine STATIC ${SOURCES})