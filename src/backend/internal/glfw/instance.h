#ifndef BACKEND_INTERNAL_GLFW_INSTANCE_H_
#define BACKEND_INTERNAL_GLFW_INSTANCE_H_

#include "engine/window/instance.h"
#include "backend/internal/glfw/error.h"

#include <GLFW/glfw3.h>

namespace glfw::internal {

class Instance : public engine::Instance {
public:
  Instance();
  ~Instance() override;
};

inline Instance::Instance() {
  if (glfwInit() == GLFW_FALSE) {
    throw Error("Couldn't init glfw");
  }
}

inline Instance::~Instance() { glfwTerminate(); }


} // namespace glfw::internal

#endif // BACKEND_INTERNAL_GLFW_INSTANCE_H_