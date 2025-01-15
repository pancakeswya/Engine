#ifndef ENGINE_WINDOW_WINDOW_LOADER_H_
#define ENGINE_WINDOW_WINDOW_LOADER_H_

#include "engine/dll_loader.h"
#include "engine/window/instance.h"
#include "engine/window/window.h"

namespace engine {

class WindowLoader final : DllLoader {
public:
  explicit WindowLoader(const std::string& path);

  [[nodiscard]] Instance::Handle LoadInstance() const;
  [[nodiscard]] Window::Handle LoadWindow(int width, int height, const std::string& title) const;

  ~WindowLoader() override = default;
};

} // namespace engine

#endif // ENGINE_WINDOW_WINDOW_LOADER_H_
