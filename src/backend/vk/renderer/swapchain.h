#ifndef BACKEND_VK_RENDERER_SWAPCHAIN_H_
#define BACKEND_VK_RENDERER_SWAPCHAIN_H_

#include "backend/vk/renderer/device.h"

#include "backend/vk/renderer/image.h"

#include <vector>

#include <vulkan/vulkan.h>

namespace vk {

class Swapchain final : public Device::Dispatchable<VkSwapchainKHR> {
public:
  static VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
  static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
  static VkExtent2D ChooseExtent(VkExtent2D extent, const VkSurfaceCapabilitiesKHR& capabilities);

  Swapchain() noexcept;
  Swapchain(const Swapchain& other) = delete;
  Swapchain(Swapchain&& other) noexcept;
  Swapchain(VkSwapchainKHR swapchain,
            VkDevice logical_device,
            VkPhysicalDevice physical_device,
            const VkAllocationCallbacks* allocator,
            VkExtent2D extent,
            VkFormat format) noexcept;

  ~Swapchain() override = default;

  Swapchain& operator=(const Swapchain& other) = delete;
  Swapchain& operator=(Swapchain&& other) noexcept;

  [[nodiscard]] VkExtent2D ImageExtent() const noexcept;
  [[nodiscard]] VkFormat ImageFormat() const noexcept;
  [[nodiscard]] VkFormat DepthImageFormat() const noexcept;

  [[nodiscard]] std::vector<Dispatchable<VkFramebuffer>> CreateFramebuffers(VkRenderPass render_pass) const;
private:
  VkExtent2D extent_;
  VkFormat format_;

  VkPhysicalDevice physical_device_;
  Image depth_image_;
  std::vector<Dispatchable<VkImageView>> image_views_;

  [[nodiscard]] Image CreateDepthImage() const;
  [[nodiscard]] std::vector<VkImage> GetImages() const;
  [[nodiscard]] std::vector<Dispatchable<VkImageView>> CreateImageViews() const;
  [[nodiscard]] Dispatchable<VkFramebuffer> CreateFramebuffer(const std::vector<VkImageView>& views, VkRenderPass render_pass) const;
};

inline VkExtent2D Swapchain::ImageExtent() const noexcept {
  return extent_;
}

inline VkFormat Swapchain::ImageFormat() const noexcept {
  return format_;
}

inline VkFormat Swapchain::DepthImageFormat() const noexcept {
  return depth_image_.Format();
}

} // namespace vk

#endif // BACKEND_VK_RENDERER_SWAPCHAIN_H_
