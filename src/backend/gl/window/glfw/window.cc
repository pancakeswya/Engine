#include "backend/gl/window/glfw/window.h"

#include <GLFW/glfw3.h>

namespace glfw::gl {

Window::Window(const int width, const int height, const std::string& title)
  : internal::Window(width, height, title) {
  glfwMakeContextCurrent(window_);
  glfwSwapInterval(1);
}

void Window::Loop(EventHandler* handler) const {
  internal::Window::Loop(handler);
  glfwSwapBuffers(window_);
}

} // namespace glfw::gl