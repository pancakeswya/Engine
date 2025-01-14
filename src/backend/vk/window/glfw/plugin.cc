#include "engine/window/plugin.h"

#include "backend/vk/window/glfw/instance.h"
#include "backend/vk/window/glfw/window.h"

engine::Instance* GetInstance() {
  return new glfw::vk::Instance();
}

void DestroyInstance(engine::Instance* instance) {
  delete instance;
}

engine::Window* CreateWindow(const int width, const int height, const std::string& title) {
  return new glfw::vk::Window(width, height, title);
}

void DestroyWindow(engine::Window* window) {
  delete window;
}