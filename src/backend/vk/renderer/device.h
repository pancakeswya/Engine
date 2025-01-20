#ifndef BACKEND_VK_RENDERER_DEVICE_H_
#define BACKEND_VK_RENDERER_DEVICE_H_

#include <vulkan/vulkan.h>

#include <vector>

#include "backend/vk/renderer/dispatchable.h"

namespace vk {

struct QueueFamilyIndices {
  uint32_t graphic, present;
};

struct SurfaceSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

class Device final : public SelfDispatchable<VkDevice> {
public:
  static SurfaceSupportDetails GetSurfaceSupport(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

  Device(VkPhysicalDevice physical_device, const QueueFamilyIndices& indices, const std::vector<const char*>& extensions, const VkAllocationCallbacks* allocator = nullptr);

  Device() noexcept;
  Device(const Device&) = delete;
  Device(Device&& other) noexcept;
  ~Device() override = default;

  Device& operator=(const Device&) = delete;
  Device& operator=(Device&&) noexcept;

  [[nodiscard]] VkFormat FindDepthFormat() const;
  [[nodiscard]] bool FormatFeatureSupported(VkFormat format, VkFormatFeatureFlagBits feature) const;

  [[nodiscard]] VkDevice GetLogical() const noexcept;
  [[nodiscard]] VkPhysicalDevice GetPhysical() const noexcept;

  [[nodiscard]] VkQueue GetGraphicsQueue() const noexcept;
  [[nodiscard]] VkQueue GetPresentQueue() const noexcept;
  [[nodiscard]] QueueFamilyIndices GetQueueFamilyIndices() const noexcept;

  [[nodiscard]] const VkAllocationCallbacks* GetAllocationCallbacks() const noexcept;
private:
  VkPhysicalDevice physical_device_;
  QueueFamilyIndices queue_family_indices_;
};

inline VkDevice Device::GetLogical() const noexcept {
  return handle_;
}

inline VkPhysicalDevice Device::GetPhysical() const noexcept {
  return physical_device_;
}

inline QueueFamilyIndices Device::GetQueueFamilyIndices() const noexcept {
  return queue_family_indices_;
}

inline VkQueue Device::GetGraphicsQueue() const noexcept {
  VkQueue graphics_queue = VK_NULL_HANDLE;
  vkGetDeviceQueue(handle_, queue_family_indices_.graphic, 0, &graphics_queue);
  return graphics_queue;
}

inline VkQueue Device::GetPresentQueue() const noexcept {
  VkQueue present_queue = VK_NULL_HANDLE;
  vkGetDeviceQueue(handle_, queue_family_indices_.present, 0, &present_queue);
  return present_queue;
}

inline const VkAllocationCallbacks* Device::GetAllocationCallbacks() const noexcept {
  return allocator_;
}


} // namespace vk

#endif // BACKEND_VK_RENDERER_DEVICE_H_