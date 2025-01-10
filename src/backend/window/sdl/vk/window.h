#ifndef BACKEND_WINDOW_SDL_WINDOW_H_
#define BACKEND_WINDOW_SDL_WINDOW_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <string>
#include <vector>

#include "backend/window/window.h"

namespace window::sdl::vk {

class SurfaceFactory final : public window::vk::SurfaceFactory {
public:
  explicit SurfaceFactory(SDL_Window* window) noexcept;
  ~SurfaceFactory() noexcept override = default;

  [[nodiscard]] VkSurfaceKHR CreateSurface(VkInstance instance, const VkAllocationCallbacks *allocator) const override;
private:
  SDL_Window* window_;
};

class Window final : public window::vk::Window {
public:
  Window(Size size, const std::string& title);
  ~Window() noexcept override = default;

  [[nodiscard]] bool ShouldClose() const noexcept override;
  void Loop(EventHandler* handler) const noexcept override;
  void WaitUntilResized() const noexcept override;

  void SetWindowResizedCallback(ResizeCallback resize_callback) noexcept override;
  void SetWindowUserPointer(void* user_ptr) noexcept override;

  [[nodiscard]] std::vector<const char*> GetExtensions() const override;
  [[nodiscard]] Size GetSize() const noexcept override;
  [[nodiscard]] const window::vk::SurfaceFactory& GetSurfaceFactory() const noexcept override;
private:
  void HandleEvents() const noexcept;

  void* user_ptr_;
  ResizeCallback resize_callback_;

  mutable bool should_close_;

  SDL_Window* window_;
  SurfaceFactory surface_factory_;
};

inline bool Window::ShouldClose() const noexcept { return should_close_; }

inline void Window::SetWindowUserPointer(void* user_ptr) noexcept {
  user_ptr_ = user_ptr;
}

inline void Window::SetWindowResizedCallback(ResizeCallback resize_callback) noexcept {
  resize_callback_ = resize_callback;
}

inline const window::vk::SurfaceFactory& Window::GetSurfaceFactory() const noexcept {
  return surface_factory_;
}

inline Size Window::GetSize() const noexcept {
  int width, height;
  SDL_GetWindowSize(window_, &width, &height);
  return {width, height};
}

} // namespace window::sdl::vk

#endif // BACKEND_WINDOW_SDL_WINDOW_H_
