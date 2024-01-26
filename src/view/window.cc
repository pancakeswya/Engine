#include "view/window.h"
#include "view/renderer.h"
#include "model/mesh_model.h"

#include <glfw3.h>

namespace engine {

Window::Window(int w, int h, std::string_view name, GLFWmonitor* monitor, GLFWwindow* window) {
  window_ = glfwCreateWindow(w, h, name.data(), monitor, window);
}

bool Window::IsInitialized() const noexcept {
  return window_ != nullptr;
}

Result Window::Poll(){
  MeshModel model;
  Renderer renderer(&model);

  glfwMakeContextCurrent(window_);

  while (!glfwWindowShouldClose(window_)) {
    if (!renderer.Render()) {
      return kFailure;
    }
    glfwSwapBuffers(window_);
    glfwPollEvents();
  }
  return kSuccess;
}

} // namespace engine