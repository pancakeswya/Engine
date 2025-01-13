#ifndef ENGINE_MODEL_H_
#define ENGINE_MODEL_H_

#include "engine/types.h"

namespace engine {

class Model {
public:
  Model();

  void SetView(int width, int height) noexcept;
  void Rotate(float degrees) noexcept;

  [[nodiscard]] const Uniforms& GetUniforms() const noexcept;
private:
  Uniforms uniforms_;

  float degrees_;
};

inline Model::Model() : uniforms_(), degrees_(0.0f) {}

inline const Uniforms& Model::GetUniforms() const noexcept {
  return uniforms_;
}

} // namespace engine

#endif // ENGINE_MODEL_H_
