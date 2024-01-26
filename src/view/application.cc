#include "view/application.h"

#include <glfw3.h>

namespace engine {

Application::Application() noexcept : is_init_(glfwInit()) {}

Application::~Application() {
  glfwTerminate();
}

bool Application::IsInitialized() const noexcept {
  return is_init_;
}

} // namespace engine