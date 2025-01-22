#ifndef BACKEND_GL_RENDERER_RENDERER_H_
#define BACKEND_GL_RENDERER_RENDERER_H_

#include <string>

#include <GL/glew.h>

#include "backend/gl/renderer/handle_object.h"
#include "backend/gl/renderer/object.h"
#include "backend/gl/renderer/window.h"
#include "backend/gl/renderer/uniform_updater.h"
#include "engine/render/model.h"
#include "engine/render/renderer.h"

namespace gl {

class Renderer final : public engine::Renderer {
public:
  explicit Renderer(Window& window);
  ~Renderer() override = default;

  void RenderFrame() override;
  void LoadModel(const std::string& path) override;
  [[nodiscard]] engine::Model& GetModel() noexcept override;
private:
  Window& window_;
  ValueObject program_;
  UniformUpdater uniform_updater_;

  Object object_;

  engine::Model model_;
};

inline engine::Model& Renderer::GetModel() noexcept {
  return model_;
}

} // namespace gl

#endif // BACKEND_GL_RENDERER_RENDERER_H_
