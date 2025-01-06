#include "app/app.h"

#include <iostream>

#include "backend/render/vk/render.h"
#include "backend/window/glfw/instance.h"
#include "backend/window/glfw/window.h"

namespace app {

render::vk::Config GetRenderConfig() {
  render::vk::Config config = {};

  config.app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  config.app_info.pApplicationName = "VulkanFun";
  config.app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  config.app_info.pEngineName = "Simple Engine";
  config.app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  config.app_info.apiVersion = VK_API_VERSION_1_0;

  config.image_settings.stbi_format = STBI_rgb_alpha;
  config.image_settings.vk_format = VK_FORMAT_R8G8B8A8_SRGB;
  config.image_settings.dummy_image_extent = VkExtent2D{16,16};

  config.instance_extensions = {
#ifdef DEBUG
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
#ifdef __APPLE__
    VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
#endif
  };
  config.device_extensions = {};

  config.frame_count = 2;

  return config;
}

int run() noexcept try {
  window::glfw::Instance::Handle window_backend = window::glfw::Instance::Init();
  window::glfw::Window window({1280, 720}, "VulkanFun");

  const render::vk::Config config = GetRenderConfig();

  render::vk::Renderer renderer(config, window);
  renderer.LoadModel("../obj/Madara Uchiha/obj/Madara_Uchiha.obj");
  const auto [width, height] = window.GetSize();

  render::ModelController& model_controller = renderer.GetModelController();
  model_controller.SetAction(render::Model::Action(&render::Model::SetView, width, height));

  while (!window.ShouldClose()) {
    window.HandleEvents();

    if (model_controller.ActionsDone())
      model_controller.SetAction(render::Model::Action(&render::Model::Rotate, 90.0));

    renderer.RenderFrame();
    model_controller.ExecuteAction();
  }
  return 0;
} catch (const std::exception& error) {
  std::cerr << error.what() << std::endl;
  return 1;
}

} // namespace app