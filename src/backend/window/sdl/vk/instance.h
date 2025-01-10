#ifndef BACKEND_WINDOW_SDL_VK_INSTANCE_H_
#define BACKEND_WINDOW_SDL_VK_INSTANCE_H_

#include "backend/window/instance.h"

namespace window::sdl::vk {

class Instance final : public window::Instance {
public:
  static Handle Init();

  ~Instance() override;
private:
  Instance();
};

} // namespace window::sdl::vk

#endif // BACKEND_WINDOW_SDL_VK_INSTANCE_H_