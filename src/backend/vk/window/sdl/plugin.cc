#include "engine/window/plugin.h"

#include "backend/vk/window/sdl/instance.h"
#include "backend/vk/window/sdl/window.h"

engine::Instance* ENGINE_CONV PluginCreateInstance() {
  return new sdl::vk::Instance();
}

void ENGINE_CONV PluginDestroyInstance(engine::Instance* instance) {
  delete instance;
}

engine::Window* ENGINE_CONV PluginCreateWindow(int width, int height, const std::string& title) {
  return new sdl::vk::Window(width, height, title);
}

void ENGINE_CONV PluginDestroyWindow(engine::Window* window) {
  delete window;
}