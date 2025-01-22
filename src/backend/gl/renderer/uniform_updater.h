#ifndef UNIFORM_UPDATER_H
#define UNIFORM_UPDATER_H

#include <GL/glew.h>

#include "engine/render/types.h"

namespace gl {

class UniformUpdater {
public:
  explicit UniformUpdater(GLuint program) noexcept;
  void Update(const engine::Uniforms& uniforms) const;
private:
  GLuint program_;

  GLint model_location_;
  GLint view_location_;
  GLint projection_location_;
};

inline UniformUpdater::UniformUpdater(const GLuint program) noexcept
  : program_(program),
    model_location_(glGetUniformLocation(program_, "ubo.model")),
    view_location_(glGetUniformLocation(program_, "ubo.view")),
    projection_location_(glGetUniformLocation(program_, "ubo.proj")) {}

} // namespace gl

#endif //UNIFORM_UPDATER_H
