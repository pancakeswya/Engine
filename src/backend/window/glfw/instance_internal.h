#ifndef BACKEND_WINDOW_GLFW_INSTANCE_INTERNAL_H_
#define BACKEND_WINDOW_GLFW_INSTANCE_INTERNAL_H_

#include "backend/window/instance.h"

namespace window::glfw::internal {

class Instance final : public window::Instance {
 public:
  static Handle Init();

  ~Instance() override;
 private:
  Instance();
};

} // namespace window::glfw::internal

#endif // BACKEND_WINDOW_GLFW_INSTANCE_INTERNAL_H_