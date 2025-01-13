#include "app/app.h"

#include <iostream>

#include "engine/renderer_loader.h"
#include "engine/window.h"
#include "engine/window_loader.h"

namespace app {

class Engine final : public engine::Window::EventHandler {
public:
  Engine(const engine::WindowLoader& window_loader, const engine::RendererLoader& renderer_loader)
    : instance_(window_loader.LoadInstance()),
      window_(window_loader.LoadWindow(1280, 720, "VulkanFun")),
      renderer_(renderer_loader.LoadRenderer(*window_)) {}

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

  engine::InstanceHandle instance_;
  engine::WindowHandle window_;
  engine::RendererHandle renderer_;
};

int run() noexcept try {
  Engine engine(
    engine::WindowLoader("/Users/pancakeswya/VulkanEngine/build/src/backend/vk/window/sdl/libsdl_vk_window-entry.dylib"),
    engine::RendererLoader("/Users/pancakeswya/VulkanEngine/build/src/backend/vk/renderer/libvk_renderer-entry.dylib")
  );
  engine.Run();
  return 0;
} catch (const std::exception& error) {
  std::cerr << error.what() << std::endl;
  return 1;
}

} // namespace app