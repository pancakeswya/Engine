#include "vk/surface.h"
#include "vk/instance.h"
#include "vk/exception.h"

#include <iostream>

#include "glfw/window.h"

namespace vk {

Surface::Surface(Instance& instance, const glfw::Window& window) : instance_(instance.Get()), surface_() {
  if (glfwCreateWindowSurface(instance_, window.window_, nullptr, &surface_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to create window surface!");
  }
}

Surface::~Surface() {
  vkDestroySurfaceKHR(instance_, surface_, nullptr);
}

} // namespace vk