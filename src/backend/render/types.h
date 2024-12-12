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

} // namespace render

#endif // BACKEND_RENDER_TYPES_H_
