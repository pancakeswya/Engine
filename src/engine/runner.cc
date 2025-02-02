#include "engine/runner.h"

#include <sstream>

namespace engine {

Runner::Runner(const RendererLoader& renderer_loader, const WindowLoader& window_loader, std::string title)
    : title_(std::move(title)),
      window_loader_(window_loader),
      renderer_loader_(renderer_loader),
      instance_(window_loader_.LoadInstance()),
      window_(window_loader_.LoadWindow(1280, 720, title_)),
      renderer_(renderer_loader_.Load(*window_)) {}

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

  std::stringstream oss;
  oss.precision(1);
  oss << title_ << " (" << std::fixed << fps << " FPS)";

  window_->SetWindowTitle(oss.str());
}

} // namespace engine