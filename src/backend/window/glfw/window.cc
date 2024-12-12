#include "backend/window/glfw/window.h"

#include "backend/window/glfw/error.h"

namespace window::glfw {

namespace {

GLFWwindow* CreateWindow(const Size size, const std::string& title) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow* window = glfwCreateWindow(size.width, size.height, title.c_str(), nullptr, nullptr);
  if (window == nullptr) {
    throw Error("Failed to create window");
  }
  return window;
}

} // namespace

SurfaceFactory::SurfaceFactory(GLFWwindow* window) noexcept : window_(window) {}

VkSurfaceKHR SurfaceFactory::CreateSurface(VkInstance instance, const VkAllocationCallbacks *allocator) const {
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  if (const VkResult result = glfwCreateWindowSurface(instance, window_, allocator, &surface); result != VK_SUCCESS) {
    throw Error("failed to create window surface with code: " + std::to_string(result));
  }
  return surface;
}

Window::Window(const Size size, const std::string& title)
  : opaque_(), window_(CreateWindow(size, title)), surface_factory_(window_) {}

void Window::HandleEvents() const noexcept {
  glfwPollEvents();
}

bool Window::ShouldClose() const noexcept { return glfwWindowShouldClose(window_); }

void Window::WaitUntilResized() const noexcept {
  int width = 0, height = 0;
  glfwGetFramebufferSize(window_, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window_, &width, &height);
    glfwWaitEvents();
  }
}

void Window::SetWindowUserPointer(void* user_ptr) noexcept {
  opaque_.user_ptr = user_ptr;
  glfwSetWindowUserPointer(window_, &opaque_);
}

void Window::SetWindowResizedCallback(ResizeCallback resize_callback) noexcept {
  opaque_.resize_callback = resize_callback;
  glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* window, int width, int height) {
    auto opaque = static_cast<Opaque*>(glfwGetWindowUserPointer(window));
    opaque->resize_callback(opaque->user_ptr, {width, height});
  });
}

} // namespace window::glfw