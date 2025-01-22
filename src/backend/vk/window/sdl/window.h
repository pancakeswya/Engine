#ifndef BACKEND_VK_WINDOW_SDL_WINDOW_H_
#define BACKEND_VK_WINDOW_SDL_WINDOW_H_

#include <SDL2/SDL.h>

#include <string>
#include <vector>

#include "backend/vk/renderer/window.h"
#include "backend/internal/sdl/window.h"

namespace sdl::vk {

class SurfaceFactory final : public ::vk::SurfaceFactory {
public:
  explicit SurfaceFactory(SDL_Window* window) noexcept;
  ~SurfaceFactory() noexcept override = default;

  [[nodiscard]] VkSurfaceKHR CreateSurface(VkInstance instance, const VkAllocationCallbacks *allocator) const override;
private:
  SDL_Window* window_;
};

class Window final : public internal::Window, public ::vk::Window {
public:
  Window(int width, int height, const std::string& title);
  ~Window() noexcept override = default;

  void WaitUntilResized() const noexcept override;
  [[nodiscard]] std::vector<const char*> GetExtensions() const override;
  [[nodiscard]] const ::vk::SurfaceFactory& GetSurfaceFactory() const noexcept override;
  void OnWindowResize(int window_width, int window_height) const override;
private:
  SurfaceFactory surface_factory_;
};

inline const ::vk::SurfaceFactory& Window::GetSurfaceFactory() const noexcept {
  return surface_factory_;
}

} // namespace window::sdl::vk

#endif // BACKEND_VK_WINDOW_SDL_WINDOW_H_
