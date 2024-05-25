#include "vk/images.h"
#include "vk/exception.h"

namespace vk {

Images::Images(VkDevice logical_device, VkSwapchainKHR swapchain, VkFormat format) : logical_device_(logical_device) {
  uint32_t image_count;
  vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count, nullptr);
  images_.resize(image_count);
  vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count, images_.data());
  views_.resize(images_.size());

  for (size_t i = 0; i < images_.size(); i++) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = images_[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(logical_device_, &createInfo, nullptr, &views_[i]) != VK_SUCCESS) {
      THROW_UNEXPECTED("failed to create image views!");
    }
  }
}

Images::~Images() {
  for (auto view : views_) {
    vkDestroyImageView(logical_device_, view, nullptr);
  }
}

} // namespace vk