#include "app/app.h"

#include <iostream>

#include "backend/render/factory.h"
#include "backend/window/factory.h"

#include "backend/window/glfw/vk/instance.h"
#include "backend/window/glfw/vk/window.h"
#include "backend/render/vk/render.h"

namespace app {

class Engine final : window::EventHandler {
public:
  Engine(const window::Factory& window_factory, const render::Factory& renderer_factory)
    : instance_(window_factory.CreateInstance()),
      window_(window_factory.CreateWindow({1280, 720}, "VulkanFun")),
      renderer_(renderer_factory.CreateRenderer(*window_)) {}

  ~Engine() override = default;

  void Run() {
    renderer_->LoadModel("../obj/Madara Uchiha/obj/Madara_Uchiha.obj");
    const auto [width, height] = window_->GetSize();

    renderer_->GetModel().SetView(width, height);
    while (!window_->ShouldClose()) {
      window_->Loop(this);
    }
  }
private:
  void OnRenderEvent() override {
    renderer_->GetModel().Rotate(1.0);
    renderer_->RenderFrame();
  }

  window::Instance::Handle instance_;
  window::Window::Handle window_;
  render::Renderer::Handle renderer_;
};

int run() noexcept try {
  Engine engine{
    window::vk::Factory(window::Type::kGlfw),
    render::Factory(render::Type::kGl)
  };
  engine.Run();
  return 0;
} catch (const std::exception& error) {
  std::cerr << error.what() << std::endl;
  return 1;
}

} // namespace app