#ifndef BACKEND_WINDOW_GLFW_INSTANCE_INTERNAL_H_
#define BACKEND_WINDOW_GLFW_INSTANCE_INTERNAL_H_

#include "backend/window/instance.h"
#include "backend/window/glfw/error.h"
#include "entity/singleton.h"

namespace window::glfw::internal {

class Instance final : public entity::Singleton<Instance, Error>, public window::Instance {
 public:
  ~Instance() override;
 private:
  friend class Singleton;

  Instance();
};

} // namespace window::glfw::internal

#endif // BACKEND_WINDOW_GLFW_INSTANCE_INTERNAL_H_