#ifndef BACKEND_VK_WINDOW_SDL_INSTANCE_H_
#define BACKEND_VK_WINDOW_SDL_INSTANCE_H_

#include "backend/internal/sdl/instance.h"

namespace sdl::vk {

class Instance final : public internal::Instance{
public:
  Instance();
  ~Instance() override;
};

} // namespace sdl::vk

#endif // BACKEND_VK_WINDOW_SDL_INSTANCE_H_