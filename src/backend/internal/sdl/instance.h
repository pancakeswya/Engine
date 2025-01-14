#ifndef BACKEND_INTERNAL_SDL_INSTANCE_H_
#define BACKEND_INTERNAL_SDL_INSTANCE_H_

#include "engine/window/instance.h"

namespace sdl::internal {

class Instance : public virtual engine::Instance {
public:
  Instance();
  ~Instance() override;
};

} // namespace Internal::sdl

#endif // BACKEND_INTERNAL_SDL_INSTANCE_H_