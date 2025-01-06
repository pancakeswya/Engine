#include "backend/window/sdl/instance.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "backend/window/sdl/error.h"

namespace window::sdl {

Instance::Handle Instance::Init() {
  static Instance* instance_ptr = nullptr;
  if (instance_ptr == nullptr) {
    instance_ptr = new Instance;
    return Handle(instance_ptr);
  }
  throw Error("sdl instance already created");
}

Instance::Instance() {
  if (const int res = SDL_Init(SDL_INIT_VIDEO); res != 0) {
    throw Error("sdl init failed").WithCode(res);
  }
  if (const int res = SDL_Vulkan_LoadLibrary(nullptr); res != 0) {
    throw Error("sdl load vulkan failed").WithCode(res);
  }
}

Instance::~Instance() {
  SDL_Vulkan_UnloadLibrary();
  SDL_Quit();
}

} // namespace window::sdl