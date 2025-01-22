#ifndef BACKEND_VK_RENDERER_SWAPCHAIN_H_
#define BACKEND_VK_RENDERER_SWAPCHAIN_H_

#include <vector>

#include <vulkan/vulkan.h>

#include "backend/vk/renderer/dispatchable.h"

namespace vk {

class Swapchain : public DeviceDispatchable<VkSwapchainKHR> {
public:
  using DeviceDispatchable::DeviceDispatchable;

  [[nodiscard]] std::vector<VkImage> GetImages() const;
  [[nodiscard]] VkExtent2D GetExtent() const noexcept;
  [[nodiscard]] VkFormat GetFormat() const noexcept;
private:
  friend class Device;

  VkExtent2D extent_;
  VkFormat format_;

  explicit Swapchain(DeviceDispatchable&& swapchain, const VkExtent2D extent, const VkFormat format) noexcept
    : DeviceDispatchable(std::move(swapchain)), extent_(extent), format_(format) {}
};

inline VkExtent2D Swapchain::GetExtent() const noexcept {
  return extent_;
}

inline VkFormat Swapchain::GetFormat() const noexcept {
  return format_;
}

} // namespace vk

#endif // BACKEND_VK_RENDERER_SWAPCHAIN_H_
