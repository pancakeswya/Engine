#ifndef BACKEND_VK_RENDERER_INSTANCE_H_
#define BACKEND_VK_RENDERER_INSTANCE_H_

#include <vector>

#include <vulkan/vulkan.h>

#include "backend/vk/renderer/dispatchable.h"
#include "backend/vk/renderer/window.h"

namespace vk {

class Instance final : public SelfDispatchable<VkInstance> {
public:
#ifdef DEBUG
  static std::vector<const char*> GetLayers();
  static VkDebugUtilsMessengerCreateInfoEXT GetMessengerCreateInfo() noexcept;
#endif

  explicit Instance(const VkApplicationInfo& app_info, const std::vector<const char*>& extensions, const VkAllocationCallbacks* allocator = nullptr);

#ifdef DEBUG
  [[nodiscard]] InstanceDispatchable<VkDebugUtilsMessengerEXT> CreateMessenger() const;
#endif
  [[nodiscard]] InstanceDispatchable<VkSurfaceKHR> CreateSurface(const Window& window) const;

  [[nodiscard]] std::vector<VkPhysicalDevice> EnumeratePhysicalDevices() const;
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_INSTANCE_H_