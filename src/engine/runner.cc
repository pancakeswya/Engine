#include "engine/runner.h"

namespace engine {

Runner::Runner(const WindowLoader& window_loader, const RendererLoader& renderer_loader)
    : instance_(window_loader.LoadInstance()),
      window_(window_loader.LoadWindow(1280, 720, "Engine")),
      renderer_(renderer_loader.Load(*window_)) {}

void Runner::Run() {
  renderer_->LoadModel("../obj/Madara Uchiha/obj/Madara_Uchiha.obj");
  renderer_->GetModel().SetView(window_->GetWidth(), window_->GetHeight());
  while (!window_->ShouldClose()) {
    window_->Loop(this);
  }
}

void Runner::OnRenderEvent() {
  renderer_->GetModel().Rotate(1.0);
  renderer_->RenderFrame();
}

} // namespace engine