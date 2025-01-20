#ifndef BACKEND_VK_RENDERER_SWAPCHAIN_H_
#define BACKEND_VK_RENDERER_SWAPCHAIN_H_

#include "backend/vk/renderer/dispatchable.h"

#include <vector>

#include <vulkan/vulkan.h>

namespace vk {

class Swapchain final : public DeviceDispatchable<VkSwapchainKHR> {
public:
  static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
  static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
  static VkExtent2D ChooseExtent(VkExtent2D extent, const VkSurfaceCapabilitiesKHR& capabilities);

  Swapchain() noexcept;
  Swapchain(const Swapchain& other) = delete;
  Swapchain(Swapchain&& other) noexcept;
  ~Swapchain() override = default;

  Swapchain& operator=(const Swapchain& other) = delete;
  Swapchain& operator=(Swapchain&& other) noexcept;

  [[nodiscard]] std::vector<VkImage> GetImages() const;
  [[nodiscard]] VkExtent2D GetExtent() const noexcept;
  [[nodiscard]] VkFormat GetFormat() const noexcept;
private:
  using Base = DeviceDispatchable<VkSwapchainKHR>;

  friend class DeviceDispatchableFactory;

  VkExtent2D extent_;
  VkFormat format_;

  VkPhysicalDevice physical_device_;

  Swapchain(DeviceDispatchable<VkSwapchainKHR>&& swapchain,
            VkPhysicalDevice physical_device,
            VkExtent2D extent,
            VkFormat format) noexcept;
};

inline VkExtent2D Swapchain::GetExtent() const noexcept {
  return extent_;
}

inline VkFormat Swapchain::GetFormat() const noexcept {
  return format_;
}

} // namespace vk

#endif // BACKEND_VK_RENDERER_SWAPCHAIN_H_
