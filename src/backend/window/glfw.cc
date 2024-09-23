#include "backend/window/glfw.h"

namespace glfw {

Backend::Instance Backend::Init() {
  static Backend* instance = nullptr;
  if (instance == nullptr) {
    instance = new Backend;
    return Instance(instance);
  }
  throw Error("glfw instance already created");
}

Backend::Backend() {
  if (glfwInit() == GLFW_FALSE) {
    throw Error("Couldn't init glfw");
  }
}

Backend::~Backend() {
  glfwTerminate();
}

GLFWwindow* CreateWindow(const int width, const int height, const char* title) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (window == nullptr) {
    throw Error("Failed to create window");
  }
  return window;
}

} // namespace glfw