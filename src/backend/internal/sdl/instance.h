#ifndef BACKEND_INTERNAL_SDL_INSTANCE_H_
#define BACKEND_INTERNAL_SDL_INSTANCE_H_

#include "engine/window/instance.h"
#include "backend/internal/sdl/error.h"

#include <SDL2/SDL.h>

namespace sdl::internal {

class Instance : public virtual engine::Instance {
public:
  Instance();
  ~Instance() override;
};

inline Instance::Instance() {
  if (const int res = SDL_Init(SDL_INIT_VIDEO); res != 0) {
    throw Error("sdl init failed").WithCode(res);
  }
}

inline Instance::~Instance() { SDL_Quit(); }

} // namespace Internal::sdl

#endif // BACKEND_INTERNAL_SDL_INSTANCE_H_