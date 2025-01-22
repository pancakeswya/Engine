#include "backend/gl/window/sdl/window.h"

namespace sdl::gl {

Window::Window(const int width, const int height, const std::string& title)
  : internal::Window(width, height, title, SDL_WINDOW_OPENGL), context_(SDL_GL_CreateContext(window_)) {
  SDL_GL_MakeCurrent(window_, context_);
  SDL_GL_SetSwapInterval(1);
}

void Window::Loop() const noexcept {
  internal::Window::Loop();
  SDL_GL_SwapWindow(window_);
}

void Window::OnWindowResize([[maybe_unused]]const int window_width, [[maybe_unused]]const int window_height) const {
  int width, height;
  SDL_GL_GetDrawableSize(window_, &width, &height);
  internal::Window::OnWindowResize(width, height);
}

} // namespace sdl::gl