#include "engine/window/window_loader.h"

#include "engine/window/plugin.h"

namespace engine {

WindowLoader::WindowLoader(const std::string& path) : DllLoader(path) {}

Instance::Handle WindowLoader::LoadInstance() const {
  const auto get_instance_proc = DllLoader::Load<decltype(&GetInstance)>("GetInstance");
  const auto destroy_instance = DllLoader::Load<decltype(&DestroyInstance)>("DestroyInstance");
  return {get_instance_proc(), destroy_instance};
}

Window::Handle WindowLoader::LoadWindow(const int width, const int height, const std::string& title) const {
  const auto create_window = DllLoader::Load<decltype(&CreateWindow)>("CreateWindow");
  const auto destroy_window = DllLoader::Load<decltype(&DestroyWindow)>("DestroyWindow");
  return {create_window(width, height, title), destroy_window};
}


} // namespace engine