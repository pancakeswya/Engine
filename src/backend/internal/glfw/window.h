#ifndef BACKEND_INTERNAL_GLFW_WINDOW_H_
#define BACKEND_INTERNAL_GLFW_WINDOW_H_

#include "engine/window/window.h"
#include "backend/internal/glfw/error.h"

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
  static GLFWwindow* CreateWindow(int width, int height, const std::string& title);

  struct Opaque {
    void* user_ptr;
    ResizeCallback resize_callback;
  } opaque_;

  GLFWwindow* window_;
};

inline GLFWwindow* Window::CreateWindow(const int width, const int height, const std::string& title) {
  GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (window == nullptr) {
    throw Error("Failed to create window");
  }
  return window;
}

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

inline Window::Window(const int width, const int height, const std::string& title)
  : opaque_(), window_(CreateWindow(width, height, title)) {}

inline void Window::Loop(EventHandler* handler) const {
  glfwPollEvents();
  handler->OnRenderEvent();
}

inline void Window::SetWindowUserPointer(void* user_ptr) noexcept {
  opaque_.user_ptr = user_ptr;
  glfwSetWindowUserPointer(window_, &opaque_);
}

inline void Window::SetWindowResizedCallback(ResizeCallback resize_callback) noexcept {
  opaque_.resize_callback = std::move(resize_callback);
  glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* window, int width, int height) {
    auto opaque = static_cast<Opaque*>(glfwGetWindowUserPointer(window));
    opaque->resize_callback(opaque->user_ptr, width, height);
  });
}

} // namespace glfw::internal

#endif // BACKEND_INTERNAL_GLFW_WINDOW_H_