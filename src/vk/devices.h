#ifndef VK_DEVICES_H_
#define VK_DEVICES_H_

#include <vector>
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
  void SubmitDraw(
    VkFence fence,
    VkCommandBuffer cmd_buffer,
    VkSemaphore wait_semaphore,
    VkSemaphore signal_semaphore
  );
  void SubmitPresentImage(
    uint32_t image_idx,
    VkSemaphore signal_semaphore,
    VkSwapchainKHR swapchain
  ) noexcept;

  VkDevice get_logical() noexcept;
  VkPhysicalDevice get_physical() noexcept;
 private:
  VkPhysicalDevice physical_;
  VkDevice logical_;
  VkQueue graphics_q_;
  VkQueue present_q_;
};

inline void Devices::WaitIdle() noexcept {
  vkDeviceWaitIdle(logical_);
}

inline VkDevice Devices::get_logical() noexcept { return logical_; }

inline VkPhysicalDevice Devices::get_physical() noexcept {
  return physical_;
}

} // namespace vk

#endif // VK_DEVICES_H_