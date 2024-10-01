#ifndef BACKEND_WINDOW_GLFW_H_
#define BACKEND_WINDOW_GLFW_H_

#include <GLFW/glfw3.h>

#include <memory>
#include <stdexcept>

namespace glfw {

struct Error final : std::runtime_error {
  using runtime_error::runtime_error;
};

class Backend {
 public:
  using Instance = std::unique_ptr<Backend>;

  static Instance Init();

  ~Backend();
 private:
  Backend();
};

extern GLFWwindow* CreateWindow(int width, int height, const char* title);

} // namespace glfw

#endif // BACKEND_WINDOW_GLFW_H_