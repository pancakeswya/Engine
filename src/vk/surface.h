#ifndef VK_SURFACE_H_
#define VK_SURFACE_H_

#include <vulkan/vulkan.h>

namespace glfw {

class Window;

} // namespace glfw

namespace vk {

class Instance;

class Surface {
public:
  explicit Surface(Instance& instance, const glfw::Window& window);
  ~Surface();
  [[nodiscard]] VkSurfaceKHR Get() const noexcept;
private:
  VkInstance instance_;
  VkSurfaceKHR surface_;
};

inline VkSurfaceKHR Surface::Get() const noexcept {
  return surface_;
}

} // namespace vk

#endif // VK_SURFACE_H_