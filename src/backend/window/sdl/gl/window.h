#ifndef BACKEND_WINDOW_SDL_GL_WINDOW_H_
#define BACKEND_WINDOW_SDL_GL_WINDOW_H_

#include "backend/window/sdl/window_internal.h"

#include <SDL2/SDL_opengl.h>

namespace window::sdl::gl {

class Window final : internal::Window, public window::gl::Window {
public:
  Window(Size size, const std::string& title);
  ~Window() override;

  void Loop(EventHandler* handler) const noexcept override;
private:
  SDL_GLContext context_;
};

inline Window::Window(const Size size, const std::string& title)
  : internal::Window(size, title, SDL_WINDOW_OPENGL), context_(SDL_GL_CreateContext(window_)) {}

inline Window::~Window() { SDL_GL_DeleteContext(context_); }

inline void Window::Loop(EventHandler* handler) const noexcept {
  internal::Window::Loop(handler);
  SDL_GL_SwapWindow(window_);
  SDL_Delay(1);
}

} // namespace window::sdl::gl

#endif // BACKEND_WINDOW_SDL_GL_WINDOW_H_