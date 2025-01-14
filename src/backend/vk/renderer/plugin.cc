#include "engine/render/plugin.h"

#include "backend/vk/renderer/config.h"
#include "backend/vk/renderer/renderer.h"
#include "backend/vk/renderer/window.h"

engine::Renderer* CreateRenderer(engine::Window& window) {
  return new vk::Renderer(vk::DefaultConfig(), dynamic_cast<vk::Window&>(window));
}

void DestroyRenderer(engine::Renderer* renderer) {
  delete renderer;
}
