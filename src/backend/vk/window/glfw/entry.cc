#include "backend/vk/renderer/instance.h"
#include "backend/vk/window/glfw/instance.h"
#include "backend/vk/window/glfw/window.h"
#include "engine/window_entry.h"

engine::Instance* GetInstance() {
  return glfw::vk::Instance::GetInstance();
}

void DestroyInstance([[maybe_unused]]engine::Instance* instance) {
  glfw::vk::Instance::Destroy(dynamic_cast<glfw::vk::Instance*>(instance));
}

engine::Window* CreateWindow(const int width, const int height, const std::string& title) {
  return new glfw::vk::Window(width, height, title);
}

extern void DestroyWindow(engine::Window* window) {
  delete window;
}