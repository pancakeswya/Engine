#include "backend/vk/renderer/swapchain.h"

#include "backend/vk/renderer/error.h"

namespace vk {

std::vector<VkImage> Swapchain::GetImages() const {
  uint32_t image_count;
  if (const VkResult result = vkGetSwapchainImagesKHR(GetDevice(), GetHandle(), &image_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get swapchain image count").WithCode(result);
  }
  std::vector<VkImage> images(image_count);
  if (const VkResult result = vkGetSwapchainImagesKHR(GetDevice(), GetHandle(), &image_count, images.data()); result != VK_SUCCESS) {
    throw Error("failed to get swapchain images").WithCode(result);
  }
  return images;
}

} // namespace vk
