#ifndef BACKEND_INTERNAL_SDL_ERROR_H_
#define BACKEND_INTERNAL_SDL_ERROR_H_

#include <stdexcept>
#include <string>

#include <SDL2/SDL.h>

namespace sdl::internal {

struct Error : std::runtime_error {
  using runtime_error::runtime_error;

  [[nodiscard]] Error WithCode(const int result) const {
    return Error{std::string(what()) + " [Code: " + std::to_string(result) + ']'}.WithMessage();
  }

  [[nodiscard]] Error WithMessage() const {
    throw Error{std::string(what()) + " [Message: " + SDL_GetError() + ']'};
  }
};

} // namespace sdl::internal

#endif // BACKEND_INTERNAL_SDL_ERROR_H_
