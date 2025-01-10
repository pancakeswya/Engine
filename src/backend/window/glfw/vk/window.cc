#include "backend/window/glfw/vk/window.h"

#include "backend/window/glfw/error.h"
#include "backend/window/glfw/window_internal.h"

namespace window::glfw::vk {

SurfaceFactory::SurfaceFactory(GLFWwindow* window) noexcept : window_(window) {}

VkSurfaceKHR SurfaceFactory::CreateSurface(VkInstance instance, const VkAllocationCallbacks *allocator) const {
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  if (const VkResult result = glfwCreateWindowSurface(instance, window_, allocator, &surface); result != VK_SUCCESS) {
    throw Error("failed to create window surface with code: " + std::to_string(result));
  }
  return surface;
}

Window::Window(const Size size, const std::string& title)
  : internal::Window((glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API),
                        internal::Window(size, title))),
    surface_factory_(window_) {}

void Window::Loop(EventHandler* handler) const noexcept { internal::Window::Loop(handler); }

void Window::WaitUntilResized() const noexcept {
  int width = 0, height = 0;
  glfwGetFramebufferSize(window_, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window_, &width, &height);
    glfwWaitEvents();
  }
}

} // namespace window::glfw