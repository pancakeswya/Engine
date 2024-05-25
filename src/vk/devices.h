#ifndef VK_DEVICES_H_
#define VK_DEVICES_H_

#include <vulkan/vulkan.h>
#include <utility>

namespace vk {

namespace queue {

struct FamilyIndices {
  uint32_t graphic, present;
};

std::pair<bool, FamilyIndices> FindFamilyIndices(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
);

} // namespace queue

class Devices {
 public:
  static bool ExtensionSupport(
    VkPhysicalDevice device
  );

  explicit Devices(
    VkInstance instance,
    VkSurfaceKHR surface
  );
  ~Devices();
  void WaitIdle() noexcept;

  VkDevice Logical() noexcept;
  VkPhysicalDevice Physical() noexcept;
  VkQueue GraphicsQueue() noexcept;
  VkQueue PresentQueue() noexcept;
 private:
  VkPhysicalDevice physical_;
  VkDevice logical_;
  VkQueue graphics_q_;
  VkQueue present_q_;
};

inline void Devices::WaitIdle() noexcept {
  vkDeviceWaitIdle(logical_);
}

inline VkDevice Devices::Logical() noexcept { return logical_; }

inline VkPhysicalDevice Devices::Physical() noexcept {
  return physical_;
}

inline VkQueue Devices::GraphicsQueue() noexcept { return graphics_q_; }

inline VkQueue Devices::PresentQueue() noexcept { return present_q_; }

} // namespace vk

#endif // VK_DEVICES_H_