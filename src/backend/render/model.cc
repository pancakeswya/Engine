#include "backend/render/model.h"

#include <chrono>

namespace render {

void Model::SetView(const int width, const int height) noexcept {
  uniforms_.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  uniforms_.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 0.1f, 10.0f);
  uniforms_.proj[1][1] *= -1;
}

void Model::Rotate(const float degrees) noexcept {
  degrees_ = glm::mod(degrees_ + degrees, 360.0f);
  uniforms_.model = glm::rotate(glm::mat4(1.0f), glm::radians(degrees_), glm::vec3(0.0f, 0.0f, 1.0f));
}

} // namespace render