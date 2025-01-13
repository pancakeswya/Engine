#include "backend/internal/glfw/window.h"

#include "backend/internal/glfw/error.h"

namespace glfw::internal {

namespace {

GLFWwindow* CreateWindow(const int width, const int height, const std::string& title) {
  GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (window == nullptr) {
    throw Error("Failed to create window");
  }
  return window;
}

} // namespace

Window::Window(const int width, const int height, const std::string& title)
: opaque_(), window_(CreateWindow(width, height, title)) {}

void Window::Loop(EventHandler* handler) const {
  glfwPollEvents();
  handler->OnRenderEvent();
}

void Window::SetWindowUserPointer(void* user_ptr) noexcept {
  opaque_.user_ptr = user_ptr;
  glfwSetWindowUserPointer(window_, &opaque_);
}

void Window::SetWindowResizedCallback(ResizeCallback resize_callback) noexcept {
  opaque_.resize_callback = std::move(resize_callback);
  glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* window, int width, int height) {
    auto opaque = static_cast<Opaque*>(glfwGetWindowUserPointer(window));
    opaque->resize_callback(opaque->user_ptr, width, height);
  });
}

} // namespace glfw::internal