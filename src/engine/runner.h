#ifndef ENGINE_RUNNER_H_
#define ENGINE_RUNNER_H_

#include "engine/window/window.h"
#include "engine/render/renderer.h"

namespace engine {

class Runner final : public Window::EventHandler {
public:
  Runner(const Window::Loader& window_loader, const Renderer::Loader& renderer_loader);
  ~Runner() override = default;

  void Run();
private:
  void OnRenderEvent() override;

  Instance::Handle instance_;
  Window::Handle window_;
  Renderer::Handle renderer_;
};

} // namespace engine

#endif // ENGINE_RUNNER_H_
