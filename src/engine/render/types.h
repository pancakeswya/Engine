#ifndef ENGINE_RENDER_TYPES_H_
#define ENGINE_RENDER_TYPES_H_

#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine {

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

} // namespace engine

#endif // ENGINE_RENDER_TYPES_H_
