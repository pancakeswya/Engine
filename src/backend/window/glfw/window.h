#ifndef BACKEND_WINDOW_GLFW_WINDOW_H_
#define BACKEND_WINDOW_GLFW_WINDOW_H_

#include "backend/window/window.h"

#include <GLFW/glfw3.h>
#include <string>
#include <vector>

namespace window::glfw {

class SurfaceFactory final : public ISurfaceFactory {
public:
  explicit SurfaceFactory(GLFWwindow* window) noexcept;
  ~SurfaceFactory() noexcept override = default;

  [[nodiscard]] VkSurfaceKHR CreateSurface(VkInstance instance, const VkAllocationCallbacks *allocator) const override;
private:
  GLFWwindow* window_;
};

class Window final : public IWindow {
public:
  Window(Size size, const std::string& title);
  ~Window() noexcept override = default;

  [[nodiscard]] bool ShouldClose() const noexcept override;
  void HandleEvents() const noexcept override;
  void WaitUntilResized() const noexcept override;

  void SetWindowResizedCallback(ResizeCallback resize_callback) noexcept override;
  void SetWindowUserPointer(void* user_ptr) noexcept override;

  [[nodiscard]] std::vector<const char*> GetExtensions() const override;
  [[nodiscard]] Size GetSize() const noexcept override;
  [[nodiscard]] const ISurfaceFactory& GetSurfaceFactory() const noexcept override;
private:
  struct Opaque {
    void* user_ptr;
    ResizeCallback resize_callback;
  } opaque_;
  GLFWwindow* window_;
  SurfaceFactory surface_factory_;
};

inline std::vector<const char*> Window::GetExtensions() const {
  uint32_t ext_count = 0;
  const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&ext_count);
  return {glfw_extensions, glfw_extensions + ext_count};
}

inline const ISurfaceFactory& Window::GetSurfaceFactory() const noexcept {
  return surface_factory_;
}

inline Size Window::GetSize() const noexcept {
  int width, height;
  glfwGetFramebufferSize(window_, &width, &height);
  return {width, height};
}

} // namespace window::glfw

#endif // BACKEND_WINDOW_GLFW_WINDOW_H_
