#include "engine/runner.h"

#include <sstream>

namespace engine {

Runner::Runner(const WindowLoader& window_loader, const RendererLoader& renderer_loader)
    : instance_(window_loader.LoadInstance()),
      window_(window_loader.LoadWindow(1280, 720, "Engine")),
      renderer_(renderer_loader.Load(*window_)) {}

void Runner::Run() {
  renderer_->LoadModel("../obj/Madara Uchiha/obj/Madara_Uchiha.obj");
  renderer_->GetModel().SetView(window_->GetWidth(), window_->GetHeight());
  window_->SetWindowEventHandler(this);
  while (!window_->ShouldClose()) {
    UpdateFps();
    window_->Loop();
  }
}

void Runner::OnRenderEvent() {
  renderer_->GetModel().Rotate(1.0);
  renderer_->RenderFrame();
}

void Runner::UpdateFps() {
  const double fps = fps_counter_.Count();

  std::ostringstream oss;
  oss.precision(1);
  oss << " (" << std::fixed << fps << " FPS)";

  window_->SetWindowTitle(oss.str());

}
} // namespace engine