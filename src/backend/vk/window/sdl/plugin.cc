#include "engine/window/plugin.h"

#include "backend/vk/window/sdl/instance.h"
#include "backend/vk/window/sdl/window.h"

engine::Instance* GetInstance() {
  return new sdl::vk::Instance();
}

void DestroyInstance(engine::Instance* instance) {
  delete instance;
}

engine::Window* CreateWindow(const int width, const int height, const std::string& title) {
  return new sdl::vk::Window(width, height, title);
}

void DestroyWindow(engine::Window* window) {
  delete window;
}