#include "engine/window/window_loader.h"

#include "engine/window/plugin.h"

namespace engine {

WindowLoader::WindowLoader(const std::string& path) : DllLoader(path) {}

Instance::Handle WindowLoader::LoadInstance() const {
  const auto get_instance_proc = DllLoader::Load<decltype(&PluginCreateInstance)>("PluginCreateInstance");
  const auto destroy_instance = DllLoader::Load<decltype(&PluginDestroyInstance)>("PluginDestroyInstance");
  return {get_instance_proc(), destroy_instance};
}

Window::Handle WindowLoader::LoadWindow(const int width, const int height, const std::string& title) const {
  const auto create_window = DllLoader::Load<decltype(&PluginCreateWindow)>("PluginCreateWindow");
  const auto destroy_window = DllLoader::Load<decltype(&PluginDestroyWindow)>("PluginDestroyWindow");
  return {create_window(width, height, title), destroy_window};
}


} // namespace engine