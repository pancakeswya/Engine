#ifndef BACKEND_VK_WINDOW_GLFW_ERROR_H_
#define BACKEND_VK_WINDOW_GLFW_ERROR_H_

#include "backend/internal/glfw/error.h"

namespace glfw {

class Error final : public internal::Error {
public:
  using internal::Error::Error;
};

} // namespace glfw

#endif // BACKEND_VK_WINDOW_GLFW_ERROR_H_
