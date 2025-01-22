#include "backend/vk/window/glfw/window.h"

#include "backend/vk/window/glfw/error.h"

namespace glfw::vk {

SurfaceFactory::SurfaceFactory(GLFWwindow* window) noexcept : window_(window) {}

VkSurfaceKHR SurfaceFactory::CreateSurface(VkInstance instance, const VkAllocationCallbacks *allocator) const {
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  if (const VkResult result = glfwCreateWindowSurface(instance, window_, allocator, &surface); result != VK_SUCCESS) {
    throw Error("failed to create window surface with code: " + std::to_string(result));
  }
  return surface;
}

Window::Window(const int width, const int height, const std::string& title)
  : internal::Window(width, height, title),
    surface_factory_(window_) {}

void Window::WaitUntilResized() const noexcept {
  int width = 0, height = 0;
  glfwGetFramebufferSize(window_, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window_, &width, &height);
    glfwWaitEvents();
  }
}

} // namespace window::glfw