#ifndef BACKEND_VK_WINDOW_GLFW_INSTANCE_H_
#define BACKEND_VK_WINDOW_GLFW_INSTANCE_H_

#include "backend/internal/glfw/instance.h"

namespace glfw::vk {

class Instance final : public internal::Instance {
public:
  Instance();
  ~Instance() override = default;
};

inline Instance::Instance() {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

} // namespace window::glfw::vk

#endif // BACKEND_VK_WINDOW_GLFW_INSTANCE_H_
