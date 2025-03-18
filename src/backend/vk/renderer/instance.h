#ifndef BACKEND_VK_RENDERER_INSTANCE_H_
#define BACKEND_VK_RENDERER_INSTANCE_H_

#include <vulkan/vulkan.h>

#include <vector>
#include <optional>

#include "backend/vk/renderer/handle.h"
#include "backend/vk/renderer/window.h"

namespace vk {

class Instance final : public Handle<VkInstance> {
public:
  explicit Instance(
    const std::vector<const char*>& extensions,
    const std::vector<const char*>& layers,
    const VkAllocationCallbacks* allocator = nullptr
  );
  [[nodiscard]] InstanceHandle<VkSurfaceKHR> CreateSurface(const Window& window) const;
  [[nodiscard]] InstanceHandle<VkDebugUtilsMessengerEXT> CreateMessenger() const;

  [[nodiscard]] std::vector<VkPhysicalDevice> EnumerateDevices() const;
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_INSTANCE_H_