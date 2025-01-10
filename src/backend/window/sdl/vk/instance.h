#ifndef BACKEND_WINDOW_SDL_VK_INSTANCE_H_
#define BACKEND_WINDOW_SDL_VK_INSTANCE_H_

#include "backend/window/instance.h"
#include "backend/window/sdl/error.h"
#include "backend/window/sdl/instance_internal.h"
#include "entity/singleton.h"

namespace window::sdl::vk {

class Instance final : internal::Instance, public entity::Singleton<Instance, Error>, public window::Instance {
public:
  ~Instance() override;
private:
  friend class Singleton;

  Instance();
};

} // namespace window::sdl::vk

#endif // BACKEND_WINDOW_SDL_VK_INSTANCE_H_