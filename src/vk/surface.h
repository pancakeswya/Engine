#ifndef VK_SURFACE_H_
#define VK_SURFACE_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace glfw {

class Window;

} // namespace glfw

namespace vk {

class Instance;

class Surface {
public:
  explicit Surface(VkInstance instance, GLFWwindow* window);
  ~Surface();
  VkSurfaceKHR Get() noexcept;
private:
  VkInstance instance_;
  VkSurfaceKHR surface_;
};

inline VkSurfaceKHR Surface::Get() noexcept {
  return surface_;
}

} // namespace vk

#endif // VK_SURFACE_H_