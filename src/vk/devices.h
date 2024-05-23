#ifndef VK_DEVICE_H_
#define VK_DEVICE_H_

#include <vulkan/vulkan.h>

namespace vk {

class Devices {
 public:
  static bool ExtensionSupport(VkPhysicalDevice device);

  explicit Devices(VkInstance instance, VkSurfaceKHR surface);
  ~Devices();
  VkDevice GetLogical() noexcept;
  VkPhysicalDevice GetPhysical() noexcept;
  VkQueue GetGraphicsQueue() noexcept;
  VkQueue GetPresentQueue() noexcept;
 private:
  VkPhysicalDevice physical_;
  VkDevice logical_;
  VkQueue graphics_q_;
  VkQueue present_q_;
};

inline Devices::~Devices() { vkDestroyDevice(logical_, nullptr); }

inline VkDevice Devices::GetLogical() noexcept { return logical_; }

inline VkPhysicalDevice Devices::GetPhysical() noexcept {
  return physical_;
}

inline VkQueue Devices::GetGraphicsQueue() noexcept { return graphics_q_; }

inline VkQueue Devices::GetPresentQueue() noexcept { return present_q_; }

} // namespace device

} // namespace vk

#endif // VK_DEVICE_H_