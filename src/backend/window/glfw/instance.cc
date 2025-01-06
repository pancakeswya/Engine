#include "backend/window/glfw/instance.h"

#include <GLFW/glfw3.h>

#include "backend/window/glfw/error.h"

namespace window::glfw {

Instance::Handle Instance::Init() {
  static Instance* instance_ptr = nullptr;
  if (instance_ptr == nullptr) {
    instance_ptr = new Instance;
    return Handle(instance_ptr);
  }
  throw Error("glfw instance already created");
}

Instance::Instance() {
  if (glfwInit() == GLFW_FALSE) {
    throw Error("Couldn't init glfw");
  }
}

Instance::~Instance() {
  glfwTerminate();
}

} // namespace window::glfw