#include "engine/renderer_entry.h"

#include "backend/vk/renderer/config.h"
#include "backend/vk/renderer/renderer.h"
#include "backend/vk/window/window.h"

engine::Renderer* CreateRenderer(engine::Window& window) {
  return new vk::Renderer(vk::DefaultConfig(), dynamic_cast<vk::Window&>(window));
}
extern void DestroyRenderer(engine::Renderer* renderer) {
  delete renderer;
}
