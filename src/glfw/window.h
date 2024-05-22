#ifndef GLFW_WINDOW_H_
#define GLFW_WINDOW_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vk {

class Surface;

} // namespace vk

namespace glfw {

class Window {
 public:
  Window(const char* title, int width, int height);
  void Poll() const;
  ~Window();
 private:
  friend class vk::Surface;

  GLFWwindow* window_;
};

} // namespace glfw

#endif // GLFW_WINDOW_H_