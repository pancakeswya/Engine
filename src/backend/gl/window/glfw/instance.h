#ifndef BACKEND_GL_WINDOW_GLFW_INSTANCE_H_
#define BACKEND_GL_WINDOW_GLFW_INSTANCE_H_

#include "backend/internal/glfw/instance.h"

namespace glfw::gl {

class Instance final : public internal::Instance {
public:
  using internal::Instance::Instance;
};

} // namespace glfw::gl

#endif // BACKEND_GL_WINDOW_GLFW_INSTANCE_H_
