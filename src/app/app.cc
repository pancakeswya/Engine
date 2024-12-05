#include "app/app.h"

#include <iostream>
#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "backend/render/types.h"
#include "backend/render/vk/render.h"
#include "backend/window/glfw.h"

namespace app {

void UpdateUniforms(render::UniformBufferObject* ubo) {
  static const std::chrono::time_point start_time = std::chrono::high_resolution_clock::now();

  const std::chrono::time_point curr_time = std::chrono::high_resolution_clock::now();
  const float time = std::chrono::duration<float>(curr_time - start_time).count();

  ubo->model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo->view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo->proj = glm::perspective(glm::radians(45.0f), static_cast<float>(1280) / 720, 0.1f, 10.0f);
  ubo->proj[1][1] *= -1;
}

int run() noexcept try {
  glfw::Backend::Instance window_backend = glfw::Backend::Init();
  GLFWwindow* window = glfw::CreateWindow(1280, 720, "VulkanFun");

  vk::Render render_backend(window);
  render_backend.LoadModel("../obj/Madara Uchiha/obj/Madara_Uchiha.obj");
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    render::UniformBufferObject* ubo = render_backend.GetUBO();
    UpdateUniforms(ubo);
    render_backend.RenderFrame();
  }
  return 0;
} catch (const std::exception& error) {
  std::cerr << error.what() << std::endl;
  return 1;
}

} // namespace app