#include "backend/internal/sdl/instance.h"

#include <SDL2/SDL.h>

#include "backend/internal/sdl/error.h"

namespace sdl::internal {

Instance::Instance() {
  if (const int res = SDL_Init(SDL_INIT_VIDEO); res != 0) {
    throw Error("sdl init failed").WithCode(res);
  }
}

Instance::~Instance() { SDL_Quit(); }

} // namespace window::sdl::internal