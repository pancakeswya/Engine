#include "vk/surface.h"
#include "vk/instance.h"
#include "vk/exception.h"

namespace vk {

Surface::Surface(VkInstance instance, GLFWwindow* window) : instance_(instance), surface_() {
  if (glfwCreateWindowSurface(instance, window, nullptr, &surface_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to create window surface!");
  }
}

Surface::~Surface() {
  vkDestroySurfaceKHR(instance_, surface_, nullptr);
}

} // namespace vk