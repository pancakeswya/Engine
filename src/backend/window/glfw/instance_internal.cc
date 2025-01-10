#include "backend/window/glfw/instance_internal.h"

#include <GLFW/glfw3.h>

namespace window::glfw::internal {

Instance::Instance() {
  if (glfwInit() == GLFW_FALSE) {
    throw Error("Couldn't init glfw");
  }
}

Instance::~Instance() { glfwTerminate(); }

} // namespace window::glfw::internal