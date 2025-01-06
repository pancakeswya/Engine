#ifndef BACKEND_WINDOW_SDL_INSTANCE_H_
#define BACKEND_WINDOW_SDL_INSTANCE_H_

#include "backend/window/instance.h"

namespace window::sdl {

class Instance final : public IInstance {
public:
  static Handle Init();

  ~Instance() override;
private:
  Instance();
};

} // namespace window::sdl

#endif // BACKEND_WINDOW_SDL_INSTANCE_H_