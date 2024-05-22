#include "engine.h"
#include "base/exception.h"
#include "glfw/window.h"
#include "vk/instance.h"

#ifdef DEBUG
#include "vk/debug.h"
#endif

#include <cstdlib>
#include <iostream>

namespace engine {

int Run() noexcept {
  try {
    glfw::Window window(kTitle, kWindowWidth, kWindowHeight);
    auto vk_instance = vk::Instance::Get();
    auto vk_messenger = vk::debug::Messenger(vk_instance);
    (void)vk_messenger;
    window.Poll();
  } catch (const engine::Exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

} // namespace engine