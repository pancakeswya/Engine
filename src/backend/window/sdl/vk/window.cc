#include "backend/window/sdl/vk/window.h"

#include <SDL2/SDL_vulkan.h>

#include "backend/window/sdl/error.h"

namespace window::sdl::vk {

SurfaceFactory::SurfaceFactory(SDL_Window* window) noexcept : window_(window) {}

VkSurfaceKHR SurfaceFactory::CreateSurface(VkInstance instance, [[maybe_unused]]const VkAllocationCallbacks *allocator) const {
  VkSurfaceKHR surface = VK_NULL_HANDLE;

  if (!SDL_Vulkan_CreateSurface(window_, instance, &surface)) {
    throw Error("failed to create window surface").WithMessage();
  }
  return surface;
}

Window::Window(const Size size, const std::string& title)
  : internal::Window(size, title, SDL_WINDOW_VULKAN), surface_factory_(window_) {}

std::vector<const char*> Window::GetExtensions() const {
  uint32_t ext_count;
  if (!SDL_Vulkan_GetInstanceExtensions(window_, &ext_count, nullptr)) {
    throw Error("Failed to get instance extensions count").WithMessage();
  }
  std::vector<const char*> extensions(ext_count);
  if (!SDL_Vulkan_GetInstanceExtensions(window_, &ext_count, extensions.data())) {
    throw Error("Failed to get instance extensions").WithMessage();
  }
  return extensions;
}

void Window::WaitUntilResized() const noexcept {
  int width = 0, height = 0;
  SDL_Vulkan_GetDrawableSize(window_, &width, &height);
  while (width == 0 || height == 0) {
    SDL_Vulkan_GetDrawableSize(window_, &width, &height);
    while (SDL_WaitEvent(nullptr))
      ;
  }
}

} // namespace window::sdl::vk
