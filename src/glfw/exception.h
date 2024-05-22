#ifndef GLFW_EXCEPTION_H_
#define GLFW_EXCEPTION_H_

#include "base/exception.h"

namespace glfw {

class Exception : public engine::Exception {
 public:
  explicit Exception(const std::string& message)
      : engine::Exception("GLFW error: " + message) {}
  ~Exception() override = default;
};

} // namespace glfw

#endif // GLFW_EXCEPTION_H_