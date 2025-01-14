#ifndef ENGINE_RENDER_PLUGIN_H_
#define ENGINE_RENDER_PLUGIN_H_

#include "engine/render/renderer.h"
#include "engine/window/window.h"

extern "C" {

extern engine::Renderer* CreateRenderer(engine::Window& window);
extern void DestroyRenderer(engine::Renderer* renderer);

} // extern "C"

#endif // ENGINE_RENDER_PLUGIN_H_
