#ifndef BACKEND_WINDOW_GLFW_ERROR_H_
#define BACKEND_WINDOW_GLFW_ERROR_H_

#include <stdexcept>

namespace window::glfw {

struct Error final : std::runtime_error {
  using runtime_error::runtime_error;
};

} // namespace window::glfw

#endif // BACKEND_WINDOW_GLFW_ERROR_H_
