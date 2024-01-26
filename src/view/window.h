#ifndef ENGINE_SRC_VIEW_WINDOW_H_
#define ENGINE_SRC_VIEW_WINDOW_H_

#include "base/data_types.h"

#include <string_view>

struct GLFWwindow;
struct GLFWmonitor;

namespace engine {

class Window {
 public:
  Window(int w, int h, std::string_view name,
         GLFWmonitor* monitor = nullptr, GLFWwindow* window = nullptr);
  ~Window() = default;
  [[nodiscard]] bool IsInitialized() const noexcept;
  Result Poll();
 private:
  GLFWwindow* window_;
};

} // namespace engine

#endif // ENGINE_SRC_VIEW_WINDOW_H_