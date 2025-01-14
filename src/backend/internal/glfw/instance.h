#ifndef BACKEND_INTERNAL_GLFW_INSTANCE_H_
#define BACKEND_INTERNAL_GLFW_INSTANCE_H_

#include "engine/window/instance.h"

namespace glfw::internal {

class Instance final : public virtual engine::Instance {
public:
  Instance();
  ~Instance() override;
};

} // namespace glfw::internal

#endif // BACKEND_INTERNAL_GLFW_INSTANCE_H_