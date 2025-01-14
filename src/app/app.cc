#include "app/app.h"

#include <iostream>

#include "engine/render/renderer.h"
#include "engine/window/window.h"

namespace app {

class Engine final : public engine::Window::EventHandler {
public:
  Engine(const engine::Window::Loader& window_loader, const engine::Renderer::Loader& renderer_loader)
    : instance_(window_loader.LoadInstance()),
      window_(window_loader.LoadWindow(1280, 720, "VulkanFun")),
      renderer_(renderer_loader.Load(*window_)) {}

  ~Engine() override = default;

  void Run() {
    renderer_->LoadModel("../obj/Madara Uchiha/obj/Madara_Uchiha.obj");
    renderer_->GetModel().SetView(window_->GetWidth(), window_->GetHeight());
    while (!window_->ShouldClose()) {
      window_->Loop(this);
    }
  }
private:
  void OnRenderEvent() override {
    renderer_->GetModel().Rotate(1.0);
    renderer_->RenderFrame();
  }

  engine::Instance::Handle instance_;
  engine::Window::Handle window_;
  engine::Renderer::Handle renderer_;
};

int run() noexcept try {
  const engine::Window::Loader window_loader("/Users/pancakeswya/VulkanEngine/build/src/backend/vk/window/glfw/libglfw_vk_window-plugin.dylib");
  const engine::Renderer::Loader renderer_loader("/Users/pancakeswya/VulkanEngine/build/src/backend/vk/renderer/libvk_renderer-plugin.dylib");

  Engine engine(window_loader, renderer_loader);
  engine.Run();
  return 0;
} catch (const std::exception& error) {
  std::cerr << error.what() << std::endl;
  return 1;
}

} // namespace app