#ifndef VK_DEVICE_H_
#define VK_DEVICE_H_

#include "vulkan/vulkan.h"

#include <optional>
#include <stdexcept>
#include <vector>

namespace vk::device {

VkPhysicalDevice PhysicalFind(VkInstance instance);

class Logical {
public:
  Logical(VkInstance intance, VkPhysicalDevice physical_device);
  ~Logical();
  [[nodiscard]] VkDevice GetDevice() const noexcept;
  [[nodiscard]] VkQueue GetQueue() const noexcept;
private:
  VkDevice device_;
  VkQueue graphics_q_;
};

inline Logical::~Logical() {
  vkDestroyDevice(device_, nullptr);
}

inline VkDevice Logical::GetDevice() const noexcept {
  return device_;
}

inline VkQueue Logical::GetQueue() const noexcept {
  return graphics_q_;
}

} // namespace vk

#endif // VK_DEVICE_H_