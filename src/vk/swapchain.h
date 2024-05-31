#ifndef VK_SWAPCHAIN_H_
#define VK_SWAPCHAIN_H_

#include "base/error.h"

#include <vulkan/vulkan.h>

typedef struct VulkanDeviceInfo VulkanDeviceInfo;

typedef struct VulkanSwapchain {
  VkSwapchainKHR swapchain;
  VkFormat format;
  VkExtent2D extent;
} VulkanSwapchain;

typedef struct VulkanSwapchainImages {
  VkImage* images;
  uint32_t image_count;

  VkImageView* views;
  uint32_t view_count;

  VkFramebuffer* framebuffers;
  uint32_t framebuffer_count;
} VulkanSwapchainImages;

extern Error VulkanSwapchainCreate(VkDevice logical_device,
                                   VkSurfaceKHR surface,
                                   const VulkanDeviceInfo* info,
                                   int width, int height,
                                   VulkanSwapchain* swapchain);

extern Error VulkanSwapchainImagesCreate(VkDevice logical_device,
                                         VkRenderPass render_pass,
                                         VulkanSwapchain* swapchain,
                                         VulkanSwapchainImages* images);

extern void VulkanSwapchainDestroy(VkDevice logical_device, VulkanSwapchain* swapchain);
extern void VulkanSwapchainImagesDestroy(VkDevice logical_device, VulkanSwapchainImages* swap_images);

#endif // VK_SWAPCHAIN_H_