#include "backend/window/sdl/instance_internal.h"

#include "backend/window/sdl/error.h"

namespace window::sdl::internal {

Instance::Instance() {
  if (const int res = SDL_Init(SDL_INIT_VIDEO); res != 0) {
    throw Error("sdl init failed").WithCode(res);
  }
}

Instance::~Instance() { SDL_Quit(); }

} // namespace window::sdl::internal