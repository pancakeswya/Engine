#ifndef ENGINE_RENDERER_ENTRY_H_
#define ENGINE_RENDERER_ENTRY_H_

#include "engine/renderer.h"
#include "engine/window.h"

extern "C" {

extern engine::Renderer* CreateRenderer(engine::Window& window);
extern void DestroyRenderer(engine::Renderer* renderer);

} // extern "C"

#endif //ENGINE_RENDERER_ENTRY_H_
