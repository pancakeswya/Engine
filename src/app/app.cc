#include "app/app.h"
#include "backend/render/vk_backend.h"
#include "backend/window/glfw.h"

#include <iostream>
#include <stdexcept>

namespace app {

int run() noexcept {
  try {
    glfw::Backend::Instance window_backend = glfw::Backend::Init();
    GLFWwindow* window = glfw::CreateWindow(640, 480, "test");
    vk::Backend render_backend(window);
    render_backend.LoadModel();
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      render_backend.Render();
    }
  } catch (const std::runtime_error& error) {
    std::cerr << error.what() << std::endl;
    return 1;
  }
  return 0;
}

} // namespace app