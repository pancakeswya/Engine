#include "engine/render/plugin.h"

#include "engine/cast_util.h"
#include "backend/vk/renderer/config.h"
#include "backend/vk/renderer/renderer.h"
#include "backend/vk/renderer/window.h"

engine::Renderer* ENGINE_CONV PluginCreateRenderer(engine::Window& window) {
  return new vk::Renderer(vk::DefaultConfig(), NAMED_DYNAMIC_CAST(vk::Window&, window));
}

void ENGINE_CONV PluginDestroyRenderer(engine::Renderer* renderer) {
  delete renderer;
}
