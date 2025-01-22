#include "backend/gl/window/glfw/window.h"

#include <GLFW/glfw3.h>

namespace glfw::gl {

Window::Window(const int width, const int height, const std::string& title)
  : internal::Window(width, height, title) {
  glfwMakeContextCurrent(window_);
  glfwSwapInterval(1);
}

void Window::SetWindowEventHandler(EventHandler* handler) noexcept {
  internal::Window::SetWindowEventHandler(handler);
  glfwSetWindowRefreshCallback(window_, [](GLFWwindow* window) {
    auto opaque = static_cast<Opaque*>(glfwGetWindowUserPointer(window));
    if (opaque->event_handler_ != nullptr) {
      opaque->event_handler_->OnRenderEvent();
    }
    glfwSwapBuffers(window);
  });
}

void Window::Loop() const {
  internal::Window::Loop();
  glfwSwapBuffers(window_);
}

} // namespace glfw::gl