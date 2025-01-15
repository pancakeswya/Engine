#ifndef BACKEND_GL_RENDERER_RENDERER_H_
#define BACKEND_GL_RENDERER_RENDERER_H_

#include "engine/render/renderer.h"
#include "engine/render/model.h"
#include "backend/gl/renderer/window.h"
#include "backend/gl/renderer/handle.h"
#include "backend/gl/renderer/object.h"

#include <GL/glew.h>
#include <string>

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
  ValueHandle program_;

  Object object_;

  engine::Model model_;
};

inline engine::Model& Renderer::GetModel() noexcept {
  return model_;
}

} // namespace gl

#endif // BACKEND_GL_RENDERER_RENDERER_H_
