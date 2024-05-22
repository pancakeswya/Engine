#ifndef GLFW_WINDOW_H_
#define GLFW_WINDOW_H_

#include <GLFW/glfw3.h>

namespace glfw {

class Window {
 public:
  Window(const char* title, int width, int height);
  void Poll() const;
  ~Window();
 private:
  GLFWwindow* window_;
};

} // namespace glfw

#endif // GLFW_WINDOW_H_