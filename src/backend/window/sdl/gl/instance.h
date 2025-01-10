#ifndef BACKEND_WINDOW_SDL_GL_INSTANCE_H_
#define BACKEND_WINDOW_SDL_GL_INSTANCE_H_

#include "backend/window/sdl/error.h"
#include "backend/window/sdl/instance_internal.h"
#include "backend/window/sdl/vk/instance.h"
#include "entity/singleton.h"

namespace window::sdl::gl {

class Instance : internal::Instance, public entity::Singleton<Instance, Error>, public window::Instance {
public:
  ~Instance() override = default;
private:
  friend class Singleton;

  Instance() = default;
};

} // namespace window::sdl::gl

#endif // BACKEND_WINDOW_SDL_GL_INSTANCE_H_