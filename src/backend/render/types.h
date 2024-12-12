#ifndef BACKEND_RENDER_TYPES_H_
#define BACKEND_RENDER_TYPES_H_

#include <cstdint>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace render {

struct Vertex {
  glm::vec3 pos;
  glm::vec3 normal;
  glm::vec2 tex_coord;
};

using Index = uint32_t;

struct Uniforms {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

class Model {
public:
  explicit Model(Uniforms* uniforms);
  [[nodiscard]] bool ViewIsSet() const noexcept;
  void SetView(int width, int height) const noexcept;
  void Rotate(float degrees) const noexcept;
private:
  Uniforms* uniforms_;
  mutable bool view_is_set_;
};

inline Model::Model(Uniforms* uniforms)
  : uniforms_(uniforms), view_is_set_(false) {}

inline bool Model::ViewIsSet() const noexcept { return view_is_set_; }

} // namespace render

#endif // BACKEND_RENDER_TYPES_H_
