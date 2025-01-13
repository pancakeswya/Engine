#ifndef BACKEND_GL_RENDERER_ERROR_H_
#define BACKEND_GL_RENDERER_ERROR_H_

#include <GL/glew.h>

#include <stdexcept>
#include <string>

namespace gl {

struct Error final : std::runtime_error {
  using runtime_error::runtime_error;

  [[nodiscard]] Error WithMessage(GLuint who) const;
};

} // namespace gl

#endif // BACKEND_GL_RENDERER_ERROR_H_
