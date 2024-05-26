#include "engine.h"
#include "base/exception.h"
#include "glfw/window.h"
#include "vk/context.h"

#include <cstdlib>
#include <iostream>


namespace engine {

constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 600;
constexpr char kTitle[] = "Vulkan";

int Run() noexcept {
  try {
    glfw::Window window(kTitle, kWindowWidth, kWindowHeight);
    vk::Context context(window);
    window.Poll(context);
  } catch (const Exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

} // namespace engine