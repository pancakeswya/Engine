#ifndef ENGINE_RENDER_PLUGIN_H_
#define ENGINE_RENDER_PLUGIN_H_

#include "engine/plugin_api.h"

#include "engine/render/renderer.h"
#include "engine/window/window.h"

extern "C" {

extern ENGINE_API engine::Renderer* PluginCreateRenderer(engine::Window& window);
extern ENGINE_API void PluginDestroyRenderer(engine::Renderer* renderer);

} // extern "C"

#endif // ENGINE_RENDER_PLUGIN_H_
