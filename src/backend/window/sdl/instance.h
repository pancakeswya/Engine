#ifndef BACKEND_WINDOW_SDL_INSTANCE_H_
#define BACKEND_WINDOW_SDL_INSTANCE_H_

#include <memory>

namespace window::sdl {

class Instance {
public:
  using Handle = std::unique_ptr<Instance>;

  static Handle Init();

  ~Instance();
private:
  Instance();
};

} // namespace window::sdl

#endif // BACKEND_WINDOW_SDL_INSTANCE_H_