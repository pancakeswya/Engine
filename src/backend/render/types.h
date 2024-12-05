#ifndef BACKEND_RENDER_TYPES_H_
#define BACKEND_RENDER_TYPES_H_

#include <cstdint>
#include <glm/glm.hpp>

namespace render {

struct Vertex {
  glm::vec3 pos;
  glm::vec3 normal;
  glm::vec2 tex_coord;
};

using Index = uint32_t;

struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

} // namespace render

#endif // BACKEND_RENDER_TYPES_H_
