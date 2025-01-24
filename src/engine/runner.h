#ifndef ENGINE_RUNNER_H_
#define ENGINE_RUNNER_H_

#include <string_view>

#include "engine/window/window_loader.h"
#include "engine/render/renderer_loader.h"
#include "engine/fps_counter.h"

namespace engine {

struct RendererType {
  using Name = std::string_view;

  static constexpr Name kVk = "vk";
  static constexpr Name kGk = "gl";
};

struct WindowType {
  using Name = std::string_view;

  static constexpr Name kGlfw = "glfw";
  static constexpr Name kSdl = "sdl";
};

class Runner final : public Window::EventHandler {
public:
  Runner(RendererType::Name renderer_name, WindowType::Name window_name);
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
