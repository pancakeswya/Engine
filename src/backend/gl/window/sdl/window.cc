#include "backend/gl/window/sdl/window.h"

namespace sdl::gl {

Window::Window(const int width, const int height, const std::string& title)
  : internal::Window(width, height, title, SDL_WINDOW_OPENGL), context_(SDL_GL_CreateContext(window_)) {
  SDL_GL_MakeCurrent(window_, context_);
  SDL_GL_SetSwapInterval(1);
}

void Window::Loop(EventHandler* handler) const noexcept {
  internal::Window::Loop(handler);
  SDL_GL_SwapWindow(window_);
}

} // namespace sdl::gl