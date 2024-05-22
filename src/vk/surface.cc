#include "vk/surface.h"
#include "vk/instance.h"
#include "vk/exception.h"

#include <iostream>

#include "glfw/window.h"

namespace vk {

Surface::Surface(const glfw::Window& window) : surface_() {
  if (glfwCreateWindowSurface(Instance::Get(), window.window_, nullptr, &surface_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to create window surface!");
  }
}

Surface::~Surface() {
  std::cout << "surface" << std::endl;
  VkInstance& instance = Instance::Get();
  vkDestroySurfaceKHR(instance, surface_, nullptr);
  vkDestroyInstance(instance, nullptr);
}

} // namespace vk