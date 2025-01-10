#ifndef BACKEND_RENDER_RENDER_H_
#define BACKEND_RENDER_RENDER_H_

#include "backend/render/model.h"

#include <memory>

namespace window {

class Window;

} // namespace window

namespace render {

enum class Type {
  kVk,
  kGl
};

class Renderer {
public:
  using Handle = std::unique_ptr<Renderer>;

  virtual void RenderFrame() = 0;
  virtual void LoadModel(const std::string& path) = 0;
  virtual Model& GetModel() noexcept = 0;
  virtual ~Renderer() = default;
};

} // namespace render

#endif // BACKEND_RENDER_RENDER_H_
