#include "engine/render/plugin.h"

#include "backend/gl/renderer/renderer.h"
#include "backend/gl/renderer/window.h"

engine::Renderer* CreateRenderer(engine::Window& window) {
  return new gl::Renderer(dynamic_cast<gl::Window&>(window));
}

void DestroyRenderer(engine::Renderer* renderer) {
  delete renderer;
}