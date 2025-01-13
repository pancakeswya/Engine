
set(SOURCES
        data_util.cc
        data_util.h
        instance.h
        loader.h
        model.cc
        model.h
        renderer.h
        renderer_entry.h
        renderer_loader.h
        types.h
        window.h
        window_entry.h
        window_loader.h
)

add_library(renderer STATIC ${SOURCES})