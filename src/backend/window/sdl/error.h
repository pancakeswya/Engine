#ifndef BACKEND_WINDOW_SDL_ERROR_H_
#define BACKEND_WINDOW_SDL_ERROR_H_

#include <stdexcept>
#include <string>

#include <SDL2/SDL.h>

namespace window::sdl {

struct Error final : std::runtime_error {
  using runtime_error::runtime_error;

  [[nodiscard]] Error WithCode(const int result) const {
    return Error{std::string(what()) + " [Code: " + std::to_string(result) + ']'}.WithMessage();
  }

  [[nodiscard]] Error WithMessage() const {
    throw Error{std::string(what()) + " [Message: " + SDL_GetError() + ']'};
  }
};

} // namespace window::sdl

#endif // BACKEND_WINDOW_SDL_ERROR_H_
