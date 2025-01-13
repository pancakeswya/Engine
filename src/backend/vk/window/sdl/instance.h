#ifndef BACKEND_VK_WINDOW_SDL_INSTANCE_H_
#define BACKEND_VK_WINDOW_SDL_INSTANCE_H_

#include "backend/internal/sdl/instance.h"

#include "engine/instance.h"
#include "entity/singleton.h"

namespace sdl::vk {

class Instance final : public internal::Instance,
                       public engine::Instance,
                       public entity::Singleton<Instance> {
public:
  ~Instance() override;
private:
  friend class Singleton;

  Instance();
};

} // namespace sdl::vk

#endif // BACKEND_VK_WINDOW_SDL_INSTANCE_H_