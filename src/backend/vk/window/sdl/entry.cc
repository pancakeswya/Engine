#include "engine/window_entry.h"

#include "backend/vk/window/sdl/instance.h"
#include "backend/vk/window/sdl/window.h"

engine::Instance* GetInstance() {
  return sdl::vk::Instance::GetInstance();
}

void DestroyInstance([[maybe_unused]]engine::Instance* instance) {
  sdl::vk::Instance::Destroy(dynamic_cast<sdl::vk::Instance*>(instance));
}

engine::Window* CreateWindow(const int width, const int height, const std::string& title) {
  return new sdl::vk::Window(width, height, title);
}

extern void DestroyWindow(engine::Window* window) {
  delete window;
}