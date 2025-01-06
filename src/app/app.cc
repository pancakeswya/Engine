#include "app/app.h"

#include <iostream>

#include "backend/render/vk/render.h"
#include "backend/window/sdl/instance.h"
#include "backend/window/sdl/window.h"

namespace app {

int run() noexcept try {
  window::sdl::Instance::Handle window_backend = window::sdl::Instance::Init();
  window::sdl::Window window({1280, 720}, "VulkanFun");

  const render::vk::Config config = render::vk::DefaultConfig();

  render::vk::Renderer renderer(config, window);

  renderer.LoadModel("../obj/Madara Uchiha/obj/Madara_Uchiha.obj");
  const auto [width, height] = window.GetSize();

  render::Model& model = renderer.GetModel();
  model.SetView(width, height);
  while (!window.ShouldClose()) {
    window.HandleEvents();

    model.Rotate(1.0);
    renderer.RenderFrame();
  }
  return 0;
} catch (const std::exception& error) {
  std::cerr << error.what() << std::endl;
  return 1;
}

} // namespace app