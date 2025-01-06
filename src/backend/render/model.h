#ifndef BACKEND_RENDER_MODEL_H_
#define BACKEND_RENDER_MODEL_H_

#include "backend/render/types.h"

namespace render {

class Model {
public:
  Model() = default;

  void SetView(int width, int height) noexcept;
  void Rotate(float degrees) noexcept;

  [[nodiscard]] const Uniforms& GetUniforms() const noexcept;
private:
  Uniforms uniforms_;
};

inline const Uniforms& Model::GetUniforms() const noexcept {
  return uniforms_;
}

} // namespace render

#endif // BACKEND_RENDER_MODEL_H_
