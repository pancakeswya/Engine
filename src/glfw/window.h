#ifndef GLFW_WINDOW_H_
#define GLFW_WINDOW_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace glfw {

class Window {
 public:
  Window(const char* title, int width, int height);
  void Poll();
  GLFWwindow* Get() noexcept;
  ~Window();
 private:
  GLFWwindow* window_;
};

inline GLFWwindow* Window::Get() noexcept {
  return window_;
}

} // namespace glfw

#endif // GLFW_WINDOW_H_