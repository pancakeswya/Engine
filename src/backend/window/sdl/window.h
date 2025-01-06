#ifndef BACKEND_WINDOW_SDL_WINDOW_H_
#define BACKEND_WINDOW_SDL_WINDOW_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <string>
#include <vector>

#include "backend/window/window.h"
#include "backend/window/sdl/error.h"

namespace window::sdl {

class SurfaceFactory final : public ISurfaceFactory {
public:
  explicit SurfaceFactory(SDL_Window* window) noexcept;
  ~SurfaceFactory() noexcept override = default;

  [[nodiscard]] VkSurfaceKHR CreateSurface(VkInstance instance, const VkAllocationCallbacks *allocator) const override;
private:
  SDL_Window* window_;
};

class Window final : public IWindow {
public:
  Window(Size size, const std::string& title);
  ~Window() noexcept override = default;

  [[nodiscard]] bool ShouldClose() const noexcept override;
  void HandleEvents() const noexcept override;
  void WaitUntilResized() const noexcept override;

  void SetWindowResizedCallback(ResizeCallback resize_callback) noexcept override;
  void SetWindowUserPointer(void* user_ptr) noexcept override;

  [[nodiscard]] std::vector<const char*> GetExtensions() const override;
  [[nodiscard]] Size GetSize() const noexcept override;
  [[nodiscard]] const ISurfaceFactory& GetSurfaceFactory() const noexcept override;
private:
  struct Opaque {
    void* user_ptr;
    ResizeCallback resize_callback;
  } opaque_;

  mutable bool should_close_;

  SDL_Window* window_;
  SurfaceFactory surface_factory_;
};

inline std::vector<const char*> Window::GetExtensions() const {
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

inline const ISurfaceFactory& Window::GetSurfaceFactory() const noexcept {
  return surface_factory_;
}

inline Size Window::GetSize() const noexcept {
  int width, height;
  SDL_GetWindowSize(window_, &width, &height);
  return {width, height};
}

} // namespace window::sdl

#endif // BACKEND_WINDOW_SDL_WINDOW_H_
