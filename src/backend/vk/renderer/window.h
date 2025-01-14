#ifndef BACKEND_VK_RENDERER_WINDOW_H_
#define BACKEND_VK_RENDERER_WINDOW_H_

#include "engine/window/window.h"

#include <vulkan/vulkan.h>

#include <vector>

namespace vk {

class SurfaceFactory {
public:
  virtual ~SurfaceFactory() = default;
  [[nodiscard]] virtual VkSurfaceKHR CreateSurface(VkInstance instance,
                                                   const VkAllocationCallbacks *allocator) const = 0;
};

class Window : public virtual engine::Window {
public:
  virtual void WaitUntilResized() const = 0;

  [[nodiscard]] virtual std::vector<const char*> GetExtensions() const = 0;
  [[nodiscard]] virtual const SurfaceFactory& GetSurfaceFactory() const noexcept = 0;
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_WINDOW_H_
