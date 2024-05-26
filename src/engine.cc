#include "engine.h"
#include "base/exception.h"
#include "glfw/window.h"
#include "vk/context.h"
#include "render/render.h"

#include <iostream>

namespace engine {

namespace {

constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 600;
constexpr char kTitle[] = "Vulkan";

} // namespace

int Run() noexcept {
  try {
    glfw::Window window(kTitle, kWindowWidth, kWindowHeight);
    vk::Context context(window);
    window.Poll(render::Render, context);
  } catch (const Exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

} // namespace engine