#ifndef BACKEND_VK_WINDOW_SDL_ERROR_H_
#define BACKEND_VK_WINDOW_SDL_ERROR_H_

#include "backend/internal/sdl/error.h"

namespace sdl {

class Error final : public internal::Error {
public:
  using internal::Error::Error;
};

} // namespace sdl

#endif // BACKEND_VK_WINDOW_SDL_ERROR_H_
