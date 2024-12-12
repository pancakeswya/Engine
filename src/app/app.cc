#include "app/app.h"

#include <iostream>

#include "backend/render/vk/render.h"
#include "backend/window/glfw/instance.h"
#include "backend/window/glfw/window.h"

namespace app {

int run() noexcept try {
  window::glfw::Instance::Handle window_backend = window::glfw::Instance::Init();
  window::glfw::Window window({1280, 720}, "VulkanFun");

  render::vk::Renderer renderer(window);
  renderer.LoadModel("../obj/Madara Uchiha/obj/Madara_Uchiha.obj");
  const auto [width, height] = window.GetSize();
  while (!window.ShouldClose()) {
    window.HandleEvents();
    render::Model model = renderer.GetModel();
    if (!model.ViewIsSet()) {
      model.SetView(width, height);
    }
    model.Rotate(90.0);
    renderer.RenderFrame();
  }
  return 0;
} catch (const std::exception& error) {
  std::cerr << error.what() << std::endl;
  return 1;
}

} // namespace app