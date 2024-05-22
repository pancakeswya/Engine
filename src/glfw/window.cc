#include "glfw/window.h"
#include "glfw/exception.h"

namespace glfw {

Window::Window(const char* title, int width, int height) {
  if (glfwInit() == GLFW_FALSE) {
    throw Exception("failed to init glfw");
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

void Window::Poll() const {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
  }
}

Window::~Window() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

} // namespace glfw