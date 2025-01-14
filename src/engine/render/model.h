#ifndef ENGINE_RENDER_MODEL_H_
#define ENGINE_RENDER_MODEL_H_

#include "engine/render/types.h"

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

inline void Model::SetView(const int width, const int height) noexcept {
  uniforms_.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  uniforms_.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 0.1f, 10.0f);
  uniforms_.proj[1][1] *= -1;
}

inline void Model::Rotate(const float degrees) noexcept {
  degrees_ = glm::mod(degrees_ + degrees, 360.0f);
  uniforms_.model = glm::rotate(glm::mat4(1.0f), glm::radians(degrees_), glm::vec3(0.0f, 0.0f, 1.0f));
}

} // namespace engine

#endif // ENGINE_RENDER_MODEL_H_
