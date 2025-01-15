#ifndef BACKEND_GL_WINDOW_GLFW_ERROR_H_
#define BACKEND_GL_WINDOW_GLFW_ERROR_H_

#include "backend/internal/glfw/error.h"

namespace glfw::gl {

class Error final : public internal::Error {
  using internal::Error::Error;
};

} // namespace gl

#endif // BACKEND_GL_WINDOW_GLFW_ERROR_H_
