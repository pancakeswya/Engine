#include "engine/engine.h"
#include "backend/render/vk_backend.h"
#include "backend/window/glfw.h"

#include <iostream>
#include <stdexcept>

namespace engine {

int run() noexcept {
  try {
    glfw::Backend::Instance window_backend = glfw::Backend::Init();
    GLFWwindow* window = glfw::CreateWindow(640, 480, "test");
    vk::Backend render_backend(window);
    while (!glfwWindowShouldClose(window)) {
      render_backend.Render();
      glfwPollEvents();
    }
  } catch (const std::runtime_error& error) {
    std::cerr << error.what() << std::endl;
    return 1;
  }
  return 0;
}

} // namespace engine