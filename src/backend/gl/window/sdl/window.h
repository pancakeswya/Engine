#ifndef BACKEND_GL_WINDOW_SDL_WINDOW_H_
#define BACKEND_GL_WINDOW_SDL_WINDOW_H_

#include "backend/gl/window/window.h"
#include "backend/internal/sdl/window.h"

#include <SDL2/SDL_opengl.h>

namespace sdl::gl {

class Window final : public internal::Window, public ::gl::Window {
public:
  Window(int width, int height, const std::string& title);
  ~Window() override;

  void Loop(EventHandler* handler) const noexcept override;
private:
  SDL_GLContext context_;
};

inline Window::Window(const int width, const int height, const std::string& title)
  : internal::Window(size, title, SDL_WINDOW_OPENGL), context_(SDL_GL_CreateContext(window_)) {}

inline Window::~Window() { SDL_GL_DeleteContext(context_); }

inline void Window::Loop(EventHandler* handler) const noexcept {
  internal::Window::Loop(handler);
  SDL_GL_SwapWindow(window_);
  SDL_Delay(1);
}

} // namespace sdl::gl

#endif // BACKEND_GL_WINDOW_SDL_WINDOW_H_