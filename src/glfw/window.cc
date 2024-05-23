#include "glfw/window.h"
#include "glfw/exception.h"

namespace glfw {

Window::Window(const char* title, const int width, const int height) {
  if (glfwInit() == GLFW_FALSE) {
    THROW_UNEXPECTED("failed to init glfw");
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

void Window::Poll() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
  }
}

Window::~Window() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

} // namespace glfw