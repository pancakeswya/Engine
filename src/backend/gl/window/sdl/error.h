#ifndef BACKEND_GL_WINDOW_SDL_ERROR_H_
#define BACKEND_GL_WINDOW_SDL_ERROR_H_

#include "backend/internal/sdl/error.h"

namespace sdl::gl {

class Error final : public internal::Error {
public:
  using internal::Error::Error;
};

} // namespace sdl::gl

#endif // BACKEND_GL_WINDOW_SDL_ERROR_H_
