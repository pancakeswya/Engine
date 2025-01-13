#ifndef ENGINE_RENDERER_H_
#define ENGINE_RENDERER_H_

#include "engine/model.h"

#include <memory>

namespace engine {

class Renderer {
public:
  virtual void RenderFrame() = 0;
  virtual void LoadModel(const std::string& path) = 0;
  virtual Model& GetModel() noexcept = 0;
  virtual ~Renderer() = default;
};

} // namespace render

#endif // ENGINE_RENDERER_H_
