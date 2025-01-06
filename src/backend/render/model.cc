#include "backend/render/model.h"

#include <chrono>

namespace render {

void Model::SetView(const int width, const int height) noexcept {
  uniforms_.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  uniforms_.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / height, 0.1f, 10.0f);
  uniforms_.proj[1][1] *= -1;
}

void Model::Rotate(const float degrees) noexcept {
  static const std::chrono::time_point start_time = std::chrono::high_resolution_clock::now();

  const std::chrono::time_point curr_time = std::chrono::high_resolution_clock::now();
  const float time = std::chrono::duration<float>(curr_time - start_time).count();

  uniforms_.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(degrees), glm::vec3(0.0f, 0.0f, 1.0f));
}

} // namespace render