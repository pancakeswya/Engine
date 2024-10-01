#include "app/app.h"
#include "backend/render/vk_backend.h"
#include "backend/window/glfw.h"

#include <iostream>
#include <stdexcept>

namespace app {

int run() noexcept try {
  glfw::Backend::Instance window_backend = glfw::Backend::Init();
  GLFWwindow* window = glfw::CreateWindow(1280, 720, "VulkanFun");

  vk::Backend render_backend(window);
  render_backend.LoadModel("../obj/Madara Uchiha/obj/Madara_Uchiha.obj");

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    render_backend.Render();
  }
  return 0;
} catch (const std::exception& error) {
  std::cerr << error.what() << std::endl;
  return 1;
}

} // namespace app