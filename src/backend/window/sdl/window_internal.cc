#include "backend/window/sdl/window_internal.h"

#include "backend/window/sdl/error.h"

namespace window::sdl::internal {

namespace {

SDL_Window* CreateWindow(const Size size, const std::string& title, const int flags) {
  SDL_Window* window = SDL_CreateWindow(title.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, size.width, size.height, flags);
  if (window == nullptr) {
    throw Error("Failed to create window");
  }
  return window;
}

} // namespace

Window::Window(const Size size, const std::string& title, const SDL_WindowFlags flags)
  : user_ptr_(), should_close_(false), window_(CreateWindow(size, title, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | flags)) {}

void Window::Loop(EventHandler* handler) const noexcept {
  HandleEvents();
  handler->OnRenderEvent();
}

void Window::HandleEvents() const noexcept {
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
    resize_callback_(user_ptr_, {event.window.data1, event.window.data2});
  }
}


} // namespace window::sdl::internal