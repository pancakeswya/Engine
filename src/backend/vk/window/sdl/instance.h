#ifndef BACKEND_VK_WINDOW_SDL_INSTANCE_H_
#define BACKEND_VK_WINDOW_SDL_INSTANCE_H_

#include "backend/internal/sdl/instance.h"

#include "engine/window/instance.h"

namespace sdl::vk {

class Instance final : public internal::Instance,
                       public virtual engine::Instance {
public:
  Instance();
  ~Instance() override;
};

} // namespace sdl::vk

#endif // BACKEND_VK_WINDOW_SDL_INSTANCE_H_