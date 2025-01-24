#include "engine/runner.h"

#include <sstream>

namespace engine {

namespace {


std::string GetRendererDllPath(const RendererType::Name renderer_name) {
  std::stringstream ss;
  ss << "src/backend/"
     << renderer_name
     << "/renderer/lib"
     << renderer_name
     << "_renderer.dylib";
  return ss.str();
}

std::string GetWindowDllPath(const RendererType::Name renderer_name, const WindowType::Name window_name) {
  std::stringstream ss;
  ss << "src/backend/"
     << renderer_name
     << "/window/"
     << window_name
     <<"/lib"
     << window_name
     << '_'
     << renderer_name
     << "_window.dylib";

  return ss.str();
}

std::string GetTitle(const RendererType::Name renderer_name, const WindowType::Name window_name) {
  std::stringstream ss;
  ss << window_name << " " << renderer_name << " engine";

  return ss.str();
}

} // namespace

Runner::Runner(const RendererType::Name renderer_name, const WindowType::Name window_name)
    : title_(GetTitle(renderer_name, window_name)),
      window_loader_(GetWindowDllPath(renderer_name, window_name)),
      renderer_loader_(GetRendererDllPath(renderer_name)),
      instance_(window_loader_.LoadInstance()),
      window_(window_loader_.LoadWindow(1280, 720, title_)),
      renderer_(renderer_loader_.Load(*window_)) {}

void Runner::Run() {
  renderer_->LoadModel("/Users/pancakeswya/VulkanEngine/obj/tommy/tommy.obj");
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
  oss << title_ << " (" << std::fixed << fps << " FPS)";

  window_->SetWindowTitle(oss.str());
}

} // namespace engine