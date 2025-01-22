#include "engine/runner.h"

#include <sstream>

namespace engine {

namespace {
std::vector<std::string> GetRendererNameMapping() {
  return { "vk", "gl" };
}

std::vector<std::string> GetWindowNameMapping() {
  return { "glfw", "sdl" };
}

std::string GetRendererDllPath(const RendererType renderer_type) {
  const std::string renderer_name = GetRendererNameMapping()[static_cast<int>(renderer_type)];
  std::stringstream ss;
  ss << "src/backend/"
     << renderer_name
     << "/renderer/lib"
     << renderer_name
     << "_renderer.dylib";
  return ss.str();
}

std::string GetWindowDllPath(const RendererType renderer_type, const WindowType window_type) {
  const std::string renderer_name = GetRendererNameMapping()[static_cast<int>(renderer_type)];
  const std::string window_name = GetWindowNameMapping()[static_cast<int>(window_type)];

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

std::string GetTitle(const RendererType renderer_type, WindowType window_type) {
  const std::string renderer_name = GetRendererNameMapping()[static_cast<int>(renderer_type)];
  const std::string window_name = GetWindowNameMapping()[static_cast<int>(window_type)];

  std::stringstream ss;
  ss << window_name << " " << renderer_name << " engine";

  return ss.str();
}

} // namespace

Runner::Runner(const RendererType renderer_type, const WindowType window_type)
    : title_(GetTitle(renderer_type, window_type)),
      window_loader_(GetWindowDllPath(renderer_type, window_type)),
      renderer_loader_(GetRendererDllPath(renderer_type)),
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

  std::ostringstream oss;
  oss.precision(1);
  oss << title_ << " (" << std::fixed << fps << " FPS)";

  window_->SetWindowTitle(oss.str());

}
} // namespace engine