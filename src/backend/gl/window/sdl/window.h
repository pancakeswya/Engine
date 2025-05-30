#ifndef BACKEND_GL_WINDOW_SDL_WINDOW_H_
#define BACKEND_GL_WINDOW_SDL_WINDOW_H_

#include "backend/gl/renderer/window.h"
#include "backend/internal/sdl/window.h"

#include <SDL2/SDL_opengl.h>

namespace sdl::gl {

class Window final : public internal::Window, public ::gl::Window {
public:
  Window(int width, int height, const std::string& title);
  ~Window() override;

  void Loop() const noexcept override;
protected:
  void OnWindowResize(int window_width, int window_height) const override;
private:
  SDL_GLContext context_;
};

inline Window::~Window() { SDL_GL_DeleteContext(context_); }

} // namespace sdl::gl

#endif // BACKEND_GL_WINDOW_SDL_WINDOW_H_