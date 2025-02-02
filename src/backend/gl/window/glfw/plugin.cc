#include "engine/window/plugin.h"

#include "backend/gl/window/glfw/instance.h"
#include "backend/gl/window/glfw/window.h"

engine::Instance* ENGINE_CONV PluginCreateInstance() {
  return new glfw::gl::Instance();
}

void ENGINE_CONV PluginDestroyInstance(engine::Instance* instance) {
  delete instance;
}

engine::Window* ENGINE_CONV PluginCreateWindow(int width, int height, const std::string& title) {
  return new glfw::gl::Window(width, height, title);
}

void ENGINE_CONV PluginDestroyWindow(engine::Window* window) {
  delete window;
}