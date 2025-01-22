#include "backend/gl/renderer/uniform_updater.h"

#include <glm/gtc/type_ptr.hpp>

namespace gl {

void UniformUpdater::Update(const engine::Uniforms& uniforms) const  {
  const auto& [model, view, proj] = uniforms;

  glUniformMatrix4fv(model_location_, 1, GL_FALSE, glm::value_ptr(model[0]));
  glUniformMatrix4fv(view_location_, 1, GL_FALSE, glm::value_ptr(view[0]));
  glUniformMatrix4fv(projection_location_, 1, GL_FALSE, glm::value_ptr(proj[0]));
}

} // namespace gl