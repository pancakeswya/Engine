#include "engine/window/plugin.h"

#include "backend/gl/window/sdl/instance.h"
#include "backend/gl/window/sdl/window.h"

engine::Instance* GetInstance() {
  return new sdl::gl::Instance();
}

void DestroyInstance(engine::Instance* instance) {
  delete instance;
}

engine::Window* CreateWindow(const int width, const int height, const std::string& title) {
  return new sdl::gl::Window(width, height, title);
}

void DestroyWindow(engine::Window* window) {
  delete window;
}