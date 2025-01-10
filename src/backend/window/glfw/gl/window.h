#ifndef BACKEND_WINDOW_GLFW_GL_WINDOW_H_
#define BACKEND_WINDOW_GLFW_GL_WINDOW_H_

#include "backend/window/glfw/window_internal.h"

#include <GLFW/glfw3.h>
#include <string>

namespace window::glfw::gl {

class Window final : internal::Window, public window::gl::Window {
public:
  Window(Size size, const std::string& title);
  ~Window() override = default;

  void Loop(EventHandler* handler) const override;
};

inline Window::Window(const Size size, const std::string& title)
  : internal::Window(size, title) {
  glfwMakeContextCurrent(window_);
  glfwSwapInterval(1);
}

inline void Window::Loop(EventHandler* handler) const {
  internal::Window::Loop(handler);
  glfwSwapBuffers(window_);
}

} // namespace gl

#endif // BACKEND_WINDOW_GLFW_GL_WINDOW_H_