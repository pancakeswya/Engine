#ifndef BACKEND_INTERNAL_SDL_WINDOW_H_
#define BACKEND_INTERNAL_SDL_WINDOW_H_

#include "engine/window/window.h"

#include <SDL2/SDL.h>

namespace sdl::internal {

class Window : public virtual engine::Window {
public:
  Window(int width, int height, const std::string& title, SDL_WindowFlags flags);
  ~Window() noexcept override = default;

  void Loop(EventHandler* handler) const noexcept override;

  [[nodiscard]] bool ShouldClose() const noexcept override;
  [[nodiscard]] int GetWidth() const noexcept override;
  [[nodiscard]] int GetHeight() const noexcept override;

  void SetWindowResizedCallback(ResizeCallback resize_callback) noexcept override;
  void SetWindowUserPointer(void* user_ptr) noexcept override;
protected:
  void* user_ptr_;
  ResizeCallback resize_callback_;

  mutable bool should_close_;

  SDL_Window* window_;
private:
  void HandleEvents() const noexcept;
};

inline bool Window::ShouldClose() const noexcept { return should_close_; }

inline void Window::SetWindowUserPointer(void* user_ptr) noexcept {
  user_ptr_ = user_ptr;
}

inline void Window::SetWindowResizedCallback(ResizeCallback resize_callback) noexcept {
  resize_callback_ = resize_callback;
}

inline int Window::GetWidth() const noexcept {
  int width;
  SDL_GetWindowSize(window_, &width, nullptr);
  return width;
}

inline int Window::GetHeight() const noexcept {
  int height;
  SDL_GetWindowSize(window_, nullptr, &height);
  return height;
}

} // namespace sdl::internal

#endif // BACKEND_INTERNAL_SDL_WINDOW_H_