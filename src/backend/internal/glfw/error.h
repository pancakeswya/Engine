#ifndef BACKEND_INTERNAL_GLFW_ERROR_H_
#define BACKEND_INTERNAL_GLFW_ERROR_H_

#include <stdexcept>

namespace glfw::internal {

struct Error final : std::runtime_error {
  using runtime_error::runtime_error;
};

} // namespace glfw::internal

#endif // BACKEND_INTERNAL_GLFW_ERROR_H_
