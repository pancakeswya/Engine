#include "engine/runner.h"
#include "engine/config.h"

#include <cstdlib>
#include <iostream>

int main() {
  engine::Config config(engine::RendererType::kVk, engine::WindowType::kGlfw);

  engine::WindowLoader window_loader(config.window_plugin_path);
  engine::RendererLoader renderer_loader(config.renderer_plugin_path);
  try {
    engine::Runner runner(renderer_loader, window_loader, config.title);
    runner.Run();
    return EXIT_SUCCESS;
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
  }
  return EXIT_FAILURE;
}
