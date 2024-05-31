#ifndef VK_SWAPCHAIN_H_
#define VK_SWAPCHAIN_H_

#include "base/error.h"

#include <vulkan/vulkan.h>

typedef struct VulkanDeviceInfo VulkanDeviceInfo;

typedef struct VulkanSwapchainBase {
  VkSwapchainKHR swapchain;
  VkImage* images;
  uint32_t image_count;
  VkFormat format;
  VkExtent2D extent;
} VulkanSwapchainBase;

typedef struct VulkanSwapchain {
  VulkanSwapchainBase base;

  VkImageView* image_views;
  uint32_t image_view_count;

  VkFramebuffer* framebuffers;
  uint32_t framebuffer_count;
} VulkanSwapchain;

extern Error VulkanSwapchainCreate(VkDevice logical_device,
                            const VulkanDeviceInfo* info,
                            VkSurfaceKHR surface,
                            VkRenderPass render_pass,
                            int width,
                            int height,
                            VulkanSwapchain* swapchain);
extern void VulkanSwapchainDestroy(VkDevice logical_device, VulkanSwapchain* swapchain);

#endif // VK_SWAPCHAIN_H_