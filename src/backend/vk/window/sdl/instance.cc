#include "backend/vk/window/sdl/instance.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "backend/vk/window/sdl/error.h"

namespace sdl::vk {

Instance::Instance() {
  if (const int res = SDL_Vulkan_LoadLibrary(nullptr); res != 0) {
    throw Error("sdl load vulkan failed").WithCode(res);
  }
}

Instance::~Instance() {
  SDL_Vulkan_UnloadLibrary();
}

} // namespace window::sdl::vk