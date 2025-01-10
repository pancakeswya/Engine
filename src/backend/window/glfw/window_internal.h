#ifndef BACKEND_WINDOW_GLFW_WINDOW_INTERNAL_H_
#define BACKEND_WINDOW_GLFW_WINDOW_INTERNAL_H_

#include "backend/window/window.h"

#include <string>

#include <GLFW/glfw3.h>

namespace window::glfw::internal {

class Window : public virtual window::Window {
public:
  explicit Window(Size size, const std::string& title);
  ~Window() override = default;

  [[nodiscard]] bool ShouldClose() const noexcept override;
  void Loop(EventHandler* handler) const override;

  void SetWindowResizedCallback(ResizeCallback resize_callback) noexcept override;
  void SetWindowUserPointer(void* user_ptr) noexcept override;

  [[nodiscard]] Size GetSize() const noexcept override;
protected:
  struct Opaque {
    void* user_ptr;
    ResizeCallback resize_callback;
  } opaque_;

  GLFWwindow* window_;
};

inline bool Window::ShouldClose() const noexcept { return glfwWindowShouldClose(window_); }

inline Size Window::GetSize() const noexcept {
  int width, height;
  glfwGetFramebufferSize(window_, &width, &height);
  return {width, height};
}

} // namespace window::glfw::internal

#endif // BACKEND_WINDOW_GLFW_WINDOW_INTERNAL_H_