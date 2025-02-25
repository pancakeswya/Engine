#ifndef BACKEND_VK_RENDERER_INSTANCE_H_
#define BACKEND_VK_RENDERER_INSTANCE_H_

#include <vulkan/vulkan.h>

#include <vector>

#include "backend/vk/renderer/handle.h"
#include "backend/vk/renderer/window.h"

namespace vk {

class Instance final : public Handle<VkInstance> {
public:
#ifdef DEBUG
  static std::vector<const char*> GetLayers();
  static VkDebugUtilsMessengerCreateInfoEXT GetMessengerCreateInfo() noexcept;
#endif

  explicit Instance(const std::vector<const char*>& extensions, const VkAllocationCallbacks* allocator = nullptr);

#ifdef DEBUG
  [[nodiscard]] InstanceHandle<VkDebugUtilsMessengerEXT> CreateMessenger() const;
#endif
  [[nodiscard]] InstanceHandle<VkSurfaceKHR> CreateSurface(const Window& window) const;

  [[nodiscard]] std::vector<VkPhysicalDevice> EnumeratePhysicalDevices() const;
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_INSTANCE_H_