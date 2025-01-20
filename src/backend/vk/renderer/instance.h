#ifndef BACKEND_VK_RENDERER_INSTANCE_H_
#define BACKEND_VK_RENDERER_INSTANCE_H_

#include "backend/vk/renderer/dispatchable.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace vk {

class Instance final : public SelfDispatchable<VkInstance> {
public:
#ifdef DEBUG
  static std::vector<const char*> GetLayers();
  static VkDebugUtilsMessengerCreateInfoEXT GetMessengerCreateInfo() noexcept;
#endif

  explicit Instance(const VkApplicationInfo& app_info, const std::vector<const char*>& extensions, const VkAllocationCallbacks* allocator = nullptr);

  Instance() = default;
  Instance(const Instance&) = delete;
  Instance(Instance&& other) noexcept = default;
  ~Instance() override = default;

  Instance& operator=(const Instance&) = delete;
  Instance& operator=(Instance&&) noexcept = default;

  [[nodiscard]] std::vector<VkPhysicalDevice> EnumeratePhysicalDevices() const;
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_INSTANCE_H_