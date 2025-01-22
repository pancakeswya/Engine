#ifndef ENGINE_RUNNER_H_
#define ENGINE_RUNNER_H_

#include "engine/window/window_loader.h"
#include "engine/render/renderer_loader.h"
#include "engine/fps_counter.h"

namespace engine {

enum class RendererType {
  kVk = 0,
  kGl
};

enum class WindowType {
  kGlfw = 0,
  kSdl
};

class Runner final : public Window::EventHandler {
public:
  Runner(RendererType renderer_type, WindowType window_type);
  ~Runner() override = default;

  void Run();
private:
  void OnRenderEvent() override;
  void UpdateFps();

  std::string title_;

  WindowLoader window_loader_;
  RendererLoader renderer_loader_;

  FpsCounter fps_counter_;
  Instance::Handle instance_;
  Window::Handle window_;
  Renderer::Handle renderer_;
};

} // namespace engine

#endif // ENGINE_RUNNER_H_
