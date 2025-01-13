#include "backend/internal/glfw/instance.h"

#include <GLFW/glfw3.h>

#include "backend/internal/glfw/error.h"

namespace glfw::internal {

Instance::Instance() {
  if (glfwInit() == GLFW_FALSE) {
    throw Error("Couldn't init glfw");
  }
}

Instance::~Instance() { glfwTerminate(); }

} // namespace glfw::internal
