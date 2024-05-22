#ifndef VK_SURFACE_H_
#define VK_SURFACE_H_

#include <vulkan/vulkan.h>

namespace glfw {

class Window;

} // namespace glfw

namespace vk {

class Surface {
public:
  explicit Surface(const glfw::Window& window);
  ~Surface();
  [[nodiscard]] VkSurfaceKHR Get() const noexcept;
private:
  VkSurfaceKHR surface_;
};

inline VkSurfaceKHR Surface::Get() const noexcept {
  return surface_;
}

} // namespace vk

#endif // VK_SURFACE_H_