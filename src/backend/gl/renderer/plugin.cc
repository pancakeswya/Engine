#include "engine/render/plugin.h"
#include "engine/cast_util.h"

#include "backend/gl/renderer/renderer.h"
#include "backend/gl/renderer/window.h"

engine::Renderer* ENGINE_CONV PluginCreateRenderer(engine::Window& window) {
  return new gl::Renderer(NAMED_DYNAMIC_CAST(gl::Window&, window));
}

void ENGINE_CONV PluginDestroyRenderer(engine::Renderer* renderer) {
  delete renderer;
}