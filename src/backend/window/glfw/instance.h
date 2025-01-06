#ifndef BACKEND_WINDOW_GLFW_INSTANCE_H_
#define BACKEND_WINDOW_GLFW_INSTANCE_H_

#include "backend/window/instance.h"

namespace window::glfw {

class Instance final : public IInstance {
 public:
  static Handle Init();

  ~Instance() override;
 private:
  Instance();
};

} // namespace window::glfw

#endif // BACKEND_WINDOW_GLFW_INSTANCE_H_