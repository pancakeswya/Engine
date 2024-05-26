#ifndef GLFW_WINDOW_H_
#define GLFW_WINDOW_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <functional>

#include "vk/context.h"

namespace vk {

class Context;

} // namespace vk

namespace glfw {

class Window {
 public:
  Window(const char* title, int width, int height);
  void Poll(const std::function<void(vk::Context&)>& render, vk::Context& context);
  ~Window();
 private:
  friend class vk::Context;

  GLFWwindow* window_;
};

} // namespace glfw

#endif // GLFW_WINDOW_H_