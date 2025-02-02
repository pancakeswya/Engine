#ifndef ENGINE_RUNNER_H_
#define ENGINE_RUNNER_H_

#include <string_view>

#include "engine/window/window_loader.h"
#include "engine/render/renderer_loader.h"
#include "engine/fps_counter.h"

namespace engine {

class Runner final : public Window::EventHandler {
public:
  Runner(const RendererLoader& renderer_loader, const WindowLoader& window_loader, std::string title);
  ~Runner() override = default;

  void Run();
private:
  void OnRenderEvent() override;
  void UpdateFps();

  std::string title_;

  const WindowLoader& window_loader_;
  const RendererLoader& renderer_loader_;

  FpsCounter fps_counter_;
  Instance::Handle instance_;
  Window::Handle window_;
  Renderer::Handle renderer_;
};

} // namespace engine

#endif // ENGINE_RUNNER_H_
