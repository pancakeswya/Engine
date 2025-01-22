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

  void Loop() const override;

  [[nodiscard]] bool ShouldClose() const noexcept override;
  [[nodiscard]] int GetWidth() const noexcept override;
  [[nodiscard]] int GetHeight() const noexcept override;

  void SetWindowTitle(const std::string &title) override;
  void SetWindowEventHandler(EventHandler *handler) noexcept override;
  void SetWindowResizedCallback(ResizeCallback resize_callback) noexcept override;
protected:
  EventHandler* event_handler_;
  ResizeCallback resize_callback_;

  mutable bool should_close_;

  SDL_Window* window_;

  virtual void OnWindowResize(int window_width, int window_height) const;
private:
  static SDL_Window* CreateWindow(int width, int height, const std::string& title, int flags);

  void HandleEvents() const noexcept;
};

inline bool Window::ShouldClose() const noexcept { return should_close_; }

inline void Window::SetWindowTitle(const std::string& title) {
  SDL_SetWindowTitle(window_, title.c_str());
}

inline void Window::SetWindowEventHandler(EventHandler* handler) noexcept {
  event_handler_ = handler;
}

inline void Window::SetWindowResizedCallback(ResizeCallback resize_callback) noexcept {
  resize_callback_ = std::move(resize_callback);
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
  : event_handler_(nullptr), should_close_(false), window_(CreateWindow(width, height, title, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | flags)) {}

inline void Window::Loop() const {
  HandleEvents();
  if (event_handler_ != nullptr) {
    event_handler_->OnRenderEvent();
  }
}

inline void Window::OnWindowResize(int window_width, int window_height) const {
  if (resize_callback_ != nullptr) {
    resize_callback_(window_width, window_height);
  }
}

inline void Window::HandleEvents() const noexcept {
  SDL_Event event;
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
          case SDL_WINDOWEVENT_SIZE_CHANGED:
            OnWindowResize(event.window.data1, event.window.data2);
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
  }
}

} // namespace sdl::internal

#endif // BACKEND_INTERNAL_SDL_WINDOW_H_