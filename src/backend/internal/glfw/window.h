#ifndef BACKEND_INTERNAL_GLFW_WINDOW_H_
#define BACKEND_INTERNAL_GLFW_WINDOW_H_

#include "engine/window/window.h"
#include "backend/internal/glfw/error.h"

#include <string>
#include <chrono>

#include <GLFW/glfw3.h>

namespace glfw::internal {

class Window : public virtual engine::Window {
public:
  explicit Window(int width, int height, const std::string& title);
  ~Window() override = default;

  [[nodiscard]] bool ShouldClose() const noexcept override;
  void Loop() const override;

  void SetWindowTitle(const std::string &title) override;
  void SetWindowEventHandler(EventHandler *handler) noexcept override;
  void SetWindowResizedCallback(ResizeCallback resize_callback) noexcept override;

  [[nodiscard]] int GetWidth() const noexcept override;
  [[nodiscard]] int GetHeight() const noexcept override;
protected:
  static GLFWwindow* CreateWindow(int width, int height, const std::string& title);

  struct Opaque {
    ResizeCallback resize_callback;
    EventHandler* event_handler_;
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

inline void Window::SetWindowTitle(const std::string& title) {
  glfwSetWindowTitle(window_, title.c_str());
}

inline void Window::SetWindowEventHandler(EventHandler* handler) noexcept {
  opaque_.event_handler_ = handler;
  glfwSetWindowUserPointer(window_, &opaque_);
}

inline Window::Window(const int width, const int height, const std::string& title)
  : opaque_(), window_(CreateWindow(width, height, title)) {}

inline void Window::Loop() const {
  glfwPollEvents();
  opaque_.event_handler_->OnRenderEvent();
}

inline void Window::SetWindowResizedCallback(ResizeCallback resize_callback) noexcept {
  opaque_.resize_callback = std::move(resize_callback);
  glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* window, const int width, const int height) {
    auto opaque = static_cast<Opaque*>(glfwGetWindowUserPointer(window));
    opaque->resize_callback(width, height);
  });
}

} // namespace glfw::internal

#endif // BACKEND_INTERNAL_GLFW_WINDOW_H_