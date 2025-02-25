#include "engine/render/plugin.h"

#include "engine/cast_util.h"
#include "backend/vk/renderer/renderer.h"
#include "backend/vk/renderer/window.h"

constexpr size_t kFrameCount = 2;

engine::Renderer* ENGINE_CONV PluginCreateRenderer(engine::Window& window) {
  return new vk::Renderer(NAMED_DYNAMIC_CAST(vk::Window&, window), kFrameCount);
}

void ENGINE_CONV PluginDestroyRenderer(engine::Renderer* renderer) {
  delete renderer;
}
