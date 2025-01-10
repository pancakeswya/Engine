#ifndef BACKEND_WINDOW_SDL_VK_WINDOW_H_
#define BACKEND_WINDOW_SDL_VK_WINDOW_H_

#include <SDL2/SDL.h>

#include <string>
#include <vector>

#include "backend/window/sdl/window_internal.h"

namespace window::sdl::vk {

class SurfaceFactory final : public window::vk::SurfaceFactory {
public:
  explicit SurfaceFactory(SDL_Window* window) noexcept;
  ~SurfaceFactory() noexcept override = default;

  [[nodiscard]] VkSurfaceKHR CreateSurface(VkInstance instance, const VkAllocationCallbacks *allocator) const override;
private:
  SDL_Window* window_;
};

class Window final : internal::Window, public window::vk::Window {
public:
  Window(Size size, const std::string& title);
  ~Window() noexcept override = default;

  void WaitUntilResized() const noexcept override;
  [[nodiscard]] std::vector<const char*> GetExtensions() const override;
  [[nodiscard]] const window::vk::SurfaceFactory& GetSurfaceFactory() const noexcept override;
private:
  SurfaceFactory surface_factory_;
};

inline const window::vk::SurfaceFactory& Window::GetSurfaceFactory() const noexcept {
  return surface_factory_;
}

} // namespace window::sdl::vk

#endif // BACKEND_WINDOW_SDL_VK_WINDOW_H_
