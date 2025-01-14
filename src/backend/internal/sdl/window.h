#ifndef BACKEND_INTERNAL_SDL_WINDOW_H_
#define BACKEND_INTERNAL_SDL_WINDOW_H_

#include "engine/window/window.h"
#include "backend/internal/sdl/error.h"

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
  static SDL_Window* CreateWindow(int width, int height, const std::string& title, int flags);

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

inline SDL_Window* Window::CreateWindow(const int width, const int height, const std::string& title, const int flags) {
  SDL_Window* window = SDL_CreateWindow(title.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
  if (window == nullptr) {
    throw Error("Failed to create window");
  }
  return window;
}

inline Window::Window(const int width, const int height, const std::string& title, const SDL_WindowFlags flags)
  : user_ptr_(), should_close_(false), window_(CreateWindow(width, height, title, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | flags)) {}

inline void Window::Loop(EventHandler* handler) const noexcept {
  HandleEvents();
  handler->OnRenderEvent();
}

inline void Window::HandleEvents() const noexcept {
  SDL_Event event;

  bool window_resized = false;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        should_close_ = true;
      break;
      case SDL_WINDOWEVENT:
        if (event.window.windowID != SDL_GetWindowID(window_)) {
          break;
        }
      switch (event.window.event) {
        case SDL_WINDOWEVENT_RESIZED:
        case SDL_WINDOWEVENT_SIZE_CHANGED:
          window_resized = true;
        break;
        default:
          break;
      }
      default:
        break;
    }
  }
  if (window_resized) {
    resize_callback_(user_ptr_, event.window.data1, event.window.data2);
  }
}

} // namespace sdl::internal

#endif // BACKEND_INTERNAL_SDL_WINDOW_H_