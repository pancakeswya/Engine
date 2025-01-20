#include "backend/vk/renderer/swapchain.h"

#include <algorithm>
#include <limits>

#include "backend/vk/renderer/error.h"

namespace vk {

VkSurfaceFormatKHR Swapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats) {
  for (const VkSurfaceFormatKHR& available_format : available_formats) {
    if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return available_format;
    }
  }
  return available_formats[0];
}

VkPresentModeKHR Swapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& available_present_modes) {
  for (const auto& available_present_mode : available_present_modes) {
    if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return available_present_mode;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::ChooseExtent(const VkExtent2D extent, const VkSurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }

  return {
    std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
    std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
  };
}

Swapchain::Swapchain() noexcept : extent_(), format_(VK_FORMAT_UNDEFINED), physical_device_(VK_NULL_HANDLE)  {}

Swapchain::Swapchain(Swapchain&& other) noexcept
  : Base(std::move(other)),
    extent_(other.extent_),
    format_(other.format_),
    physical_device_(other.physical_device_) {
  other.physical_device_ = VK_NULL_HANDLE;
  other.extent_ = {};
  other.format_ = {};
}

Swapchain& Swapchain::operator=(Swapchain&& other) noexcept {
  if (this != &other) {
    Base::operator=(std::move(other));
    physical_device_ = std::exchange(other.physical_device_, VK_NULL_HANDLE);
    extent_ = std::exchange(other.extent_, {});
    format_ = std::exchange(other.format_, {});
  }
  return *this;
}

Swapchain::Swapchain(DeviceDispatchable<VkSwapchainKHR>&& swapchain,
                     VkPhysicalDevice physical_device,
                     const VkExtent2D extent,
                     const VkFormat format) noexcept :
      Base(std::move(swapchain)),
      extent_(extent),
      format_(format),
      physical_device_(physical_device) {}

std::vector<VkImage> Swapchain::GetImages() const {
  uint32_t image_count;
  if (const VkResult result = vkGetSwapchainImagesKHR(parent_, handle_, &image_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get swapchain image count").WithCode(result);
  }
  std::vector<VkImage> images(image_count);
  if (const VkResult result = vkGetSwapchainImagesKHR(parent_, handle_, &image_count, images.data()); result != VK_SUCCESS) {
    throw Error("failed to get swapchain images").WithCode(result);
  }
  return images;
}

} // namespace vk
