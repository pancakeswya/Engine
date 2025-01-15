#ifndef BACKEND_GL_WINDOW_SDL_INSTANCE_H_
#define BACKEND_GL_WINDOW_SDL_INSTANCE_H_

#include "backend/internal/sdl/instance.h"

#include "engine/window/instance.h"

namespace sdl::gl {

class Instance final : public internal::Instance,
                       public virtual engine::Instance {
public:
  using internal::Instance::Instance;
};

} // namespace sdl::gl

#endif // BACKEND_GL_WINDOW_SDL_INSTANCE_H_