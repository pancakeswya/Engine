#include "glfw/window.h"
#include "glfw/exception.h"
#include "vk/context.h"

namespace glfw {

Window::Window(const char* title, const int width, const int height) {
  if (glfwInit() == GLFW_FALSE) {
    THROW_UNEXPECTED("failed to init glfw");
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

void Window::Poll(vk::Context& context) {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
    context.Render();
  }
}

Window::~Window() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

} // namespace glfw