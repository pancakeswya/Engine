#ifndef VK_DEVICE_H_
#define VK_DEVICE_H_

#include <vulkan/vulkan.h>

#include <iostream>

namespace vk {

class Instance;
class Surface;

class Device {
public:
  explicit Device(Instance& instance, Surface& surface);
  ~Device();
  [[nodiscard]] VkDevice GetLogicalDevice() const noexcept;
  [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const noexcept;
  [[nodiscard]] VkQueue GetGraphicsQueue() const noexcept;
  [[nodiscard]] VkQueue GetPresentQueue() const noexcept;
private:
  VkPhysicalDevice physical_device_;
  VkDevice logical_device_;
  VkQueue graphics_q_;
  VkQueue present_q_;
};

inline Device::~Device() {
  vkDestroyDevice(logical_device_, nullptr);
}

inline VkDevice Device::GetLogicalDevice() const noexcept {
  return logical_device_;
}

inline VkPhysicalDevice Device::GetPhysicalDevice() const noexcept {
  return physical_device_;
}

inline VkQueue Device::GetGraphicsQueue() const noexcept {
  return graphics_q_;
}

inline VkQueue Device::GetPresentQueue() const noexcept {
  return present_q_;
}

} // namespace vk

#endif // VK_DEVICE_H_