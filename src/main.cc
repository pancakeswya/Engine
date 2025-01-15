#include "engine/render/renderer_loader.h"
#include "engine/window/window_loader.h"
#include "engine/runner.h"

#include <cstdlib>
#include <iostream>

int main() {
  try {
    const engine::WindowLoader window_loader("/Users/pancakeswya/VulkanEngine/build/src/backend/gl/window/glfw/libglfw_gl_window.dylib");
    const engine::RendererLoader renderer_loader("/Users/pancakeswya/VulkanEngine/build/src/backend/gl/renderer/libgl_renderer.dylib");

    engine::Runner runner(window_loader, renderer_loader);
    runner.Run();
    return EXIT_SUCCESS;
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
  }
  return EXIT_FAILURE;
}
