#ifndef ENGINE_WINDOW_LOADER_H_
#define ENGINE_WINDOW_LOADER_H_

#include "engine/loader.h"
#include "engine/window.h"
#include "engine/window_entry.h"
#include "engine/instance.h"

#include <string>
#include <memory>

namespace engine {

using InstanceHandle = std::unique_ptr<Instance, decltype(&DestroyInstance)>;
using WindowHandle = std::unique_ptr<Window, decltype(&DestroyWindow)>;

class WindowLoader final : Loader {
public:
  explicit WindowLoader(const std::string& path);

  [[nodiscard]] InstanceHandle LoadInstance() const;
  [[nodiscard]] WindowHandle LoadWindow(int width, int height, const std::string& title) const;

  ~WindowLoader() override = default;
};

inline WindowLoader::WindowLoader(const std::string& path) : Loader(path) {}

inline InstanceHandle WindowLoader::LoadInstance() const {
  const auto get_instance_proc = GetProc<decltype(&GetInstance)>("GetInstance");
  const auto destroy_instance = GetProc<decltype(&DestroyInstance)>("DestroyInstance");
  return {get_instance_proc(), destroy_instance};
}

inline WindowHandle WindowLoader::LoadWindow(const int width, const int height, const std::string& title) const {
  const auto create_window = GetProc<decltype(&CreateWindow)>("CreateWindow");
  const auto destroy_window = GetProc<decltype(&DestroyWindow)>("DestroyWindow");
  return {create_window(width, height, title), destroy_window};
}

} // namespace engine

#endif // ENGINE_WINDOW_LOADER_H_
