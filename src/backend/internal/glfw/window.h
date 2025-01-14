#ifndef BACKEND_INTERNAL_GLFW_WINDOW_H_
#define BACKEND_INTERNAL_GLFW_WINDOW_H_

#include "engine/window/window.h"

#include <string>

#include <GLFW/glfw3.h>

namespace glfw::internal {

class Window : public virtual engine::Window {
public:
  explicit Window(int width, int height, const std::string& title);
  ~Window() override = default;

  [[nodiscard]] bool ShouldClose() const noexcept override;
  void Loop(EventHandler* handler) const override;

  void SetWindowResizedCallback(ResizeCallback resize_callback) noexcept override;
  void SetWindowUserPointer(void* user_ptr) noexcept override;

  [[nodiscard]] int GetWidth() const noexcept override;
  [[nodiscard]] int GetHeight() const noexcept override;
protected:
  struct Opaque {
    void* user_ptr;
    ResizeCallback resize_callback;
  } opaque_;

  GLFWwindow* window_;
};

inline bool Window::ShouldClose() const noexcept { return glfwWindowShouldClose(window_); }

inline int Window::GetWidth() const noexcept {
  int width;
  glfwGetFramebufferSize(window_, &width, nullptr);
  return width;
}

inline int Window::GetHeight() const noexcept {
  int height;
  glfwGetFramebufferSize(window_, nullptr, &height);
  return height;
}

} // namespace glfw::internal

#endif // BACKEND_INTERNAL_GLFW_WINDOW_H_