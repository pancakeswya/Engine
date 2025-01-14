#include "engine/window/window.h"

#include "engine/window/plugin.h"

namespace engine {

Window::Loader::Loader(const std::string& path) : DllLoader(path) {}

Instance::Handle Window::Loader::LoadInstance() const {
  const auto get_instance_proc = DllLoader::Load<decltype(&GetInstance)>("GetInstance");
  const auto destroy_instance = DllLoader::Load<decltype(&DestroyInstance)>("DestroyInstance");
  return {get_instance_proc(), destroy_instance};
}

Window::Handle Window::Loader::LoadWindow(const int width, const int height, const std::string& title) const {
  const auto create_window = DllLoader::Load<decltype(&CreateWindow)>("CreateWindow");
  const auto destroy_window = DllLoader::Load<decltype(&DestroyWindow)>("DestroyWindow");
  auto win = create_window(width, height, title);
  auto handle = Window::Handle(win, destroy_window);

  return handle;
}


} // namespace engine