#ifndef BACKEND_GL_WINDOW_SDL_INSTANCE_H_
#define BACKEND_GL_WINDOW_SDL_INSTANCE_H_

#include "backend/internal/sdl/instance.h"

#include "engine/instance.h"
#include "entity/singleton.h"

namespace sdl::gl {

class Instance final : public internal::Instance,
                       public engine::Instance,
                       public entity::Singleton<Instance>  {
public:
  ~Instance() override = default;
private:
  friend class Singleton;

  Instance() = default;
};

} // namespace sdl::gl

#endif // BACKEND_GL_WINDOW_SDL_INSTANCE_H_