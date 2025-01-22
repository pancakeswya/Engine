#include "engine/render/renderer_loader.h"
#include "engine/window/window_loader.h"
#include "engine/runner.h"

#include <cstdlib>
#include <iostream>

int main() {
  try {
    engine::Runner runner(engine::RendererType::kVk, engine::WindowType::kSdl);
    runner.Run();
    return EXIT_SUCCESS;
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
  }
  return EXIT_FAILURE;
}
