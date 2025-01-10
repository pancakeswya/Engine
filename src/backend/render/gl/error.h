#ifndef BACKEND_RENDER_GL_ERROR_H_
#define BACKEND_RENDER_GL_ERROR_H_

#include <GL/glew.h>

#include <stdexcept>
#include <string>

namespace render::gl {

struct Error final : std::runtime_error {
  using runtime_error::runtime_error;

  [[nodiscard]] Error WithMessage(GLuint who) const;
};

} // namespace render::gl

#endif // BACKEND_RENDER_GL_ERROR_H_
