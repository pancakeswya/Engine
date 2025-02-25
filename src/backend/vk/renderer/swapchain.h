#ifndef BACKEND_VK_RENDERER_SWAPCHAIN_H_
#define BACKEND_VK_RENDERER_SWAPCHAIN_H_

#include <vulkan/vulkan.h>

#include <vector>

#include "backend/vk/renderer/handle.h"

namespace vk {

class Swapchain : public DeviceHandle<VkSwapchainKHR> {
public:
  using DeviceHandle<VkSwapchainKHR>::DeviceHandle;

  [[nodiscard]] std::vector<VkImage> images() const;
  [[nodiscard]] VkExtent2D extent() const noexcept;
  [[nodiscard]] VkFormat format() const noexcept;
private:
  friend class Device;

  VkExtent2D extent_;
  VkFormat format_;

  explicit Swapchain(DeviceHandle<VkSwapchainKHR>&& swapchain, const VkExtent2D extent, const VkFormat format) noexcept
    : DeviceHandle<VkSwapchainKHR>(std::move(swapchain)), extent_(extent), format_(format) {}
};

inline VkExtent2D Swapchain::extent() const noexcept {
  return extent_;
}

inline VkFormat Swapchain::format() const noexcept {
  return format_;
}

} // namespace vk

#endif // BACKEND_VK_RENDERER_SWAPCHAIN_H_
