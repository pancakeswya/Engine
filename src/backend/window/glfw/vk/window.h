#ifndef BACKEND_WINDOW_GLFW_VK_WINDOW_H_
#define BACKEND_WINDOW_GLFW_VK_WINDOW_H_

#include "backend/window/glfw/window_internal.h"

#include <GLFW/glfw3.h>
#include <string>
#include <vector>

namespace window::glfw::vk {

class SurfaceFactory final : public window::vk::SurfaceFactory {
public:
  explicit SurfaceFactory(GLFWwindow* window) noexcept;
  ~SurfaceFactory() noexcept override = default;

  [[nodiscard]] VkSurfaceKHR CreateSurface(VkInstance instance, const VkAllocationCallbacks *allocator) const override;
private:
  GLFWwindow* window_;
};

class Window final : public internal::Window, public window::vk::Window {
public:
  Window(Size size, const std::string& title);
  ~Window() noexcept override = default;

  void Loop(EventHandler* handler) const noexcept override;
  void WaitUntilResized() const noexcept override;

  [[nodiscard]] std::vector<const char*> GetExtensions() const override;
  [[nodiscard]] const window::vk::SurfaceFactory& GetSurfaceFactory() const noexcept override;
private:
  SurfaceFactory surface_factory_;
};

inline std::vector<const char*> Window::GetExtensions() const {
  uint32_t ext_count = 0;
  const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&ext_count);
  return {glfw_extensions, glfw_extensions + ext_count};
}

inline const window::vk::SurfaceFactory& Window::GetSurfaceFactory() const noexcept {
  return surface_factory_;
}

} // namespace window::glfw::vk

#endif // BACKEND_WINDOW_GLFW_VK_WINDOW_H_
