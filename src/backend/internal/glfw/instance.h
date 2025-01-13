#ifndef BACKEND_INTERNAL_GLFW_INSTANCE_H_
#define BACKEND_INTERNAL_GLFW_INSTANCE_H_

#include "engine/instance.h"
#include "entity/singleton.h"

namespace glfw::internal {

class Instance final : public entity::Singleton<Instance>, public engine::Instance {
public:
  ~Instance() override;
private:
  friend class Singleton;

  Instance();
};

} // namespace glfw::internal

#endif // BACKEND_INTERNAL_GLFW_INSTANCE_H_