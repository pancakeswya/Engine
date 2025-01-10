#ifndef BACKEND_WINDOW_SDL_WINDOW_H_
#define BACKEND_WINDOW_SDL_WINDOW_H_

#include "backend/window/window.h"

#include <SDL2/SDL.h>

namespace window::sdl::internal {

class Window : public virtual window::Window {
public:
  Window(Size size, const std::string& title, SDL_WindowFlags flags);
  ~Window() noexcept override = default;

  void Loop(EventHandler* handler) const noexcept override;

  [[nodiscard]] bool ShouldClose() const noexcept override;
  [[nodiscard]] Size GetSize() const noexcept override;

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

inline Size Window::GetSize() const noexcept {
  int width, height;
  SDL_GetWindowSize(window_, &width, &height);
  return {width, height};
}

} // namespace window::sdl::internal

#endif // BACKEND_WINDOW_SDL_WINDOW_H_