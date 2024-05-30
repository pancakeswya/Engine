#ifndef VK_SWAPCHAIN_H_
#define VK_SWAPCHAIN_H_

#include "base/error.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct QueueFamilyIndices QueueFamilyIndices;
typedef struct SurfaceSupportDetails SurfaceSupportDetails;

typedef struct Swapchain {
  VkSwapchainKHR instance;

  VkDevice logical_device;

  VkImage* images;
  uint32_t image_count;

  VkImageView* image_views;
  uint32_t image_view_count;

  VkFramebuffer* framebuffers;
  uint32_t framebuffer_count;

  VkFormat format;
  VkExtent2D extent;
} Swapchain;

Error VulkanSwapchainCreate(GLFWwindow* window, VkDevice logical_device,
                            VkSurfaceKHR surface,
                            VkRenderPass render_pass,
                            QueueFamilyIndices* indices,
                            SurfaceSupportDetails* details,
                            Swapchain* swapchain);
void VulkanSwapchainDestroy(Swapchain* swapchain);

#endif // VK_SWAPCHAIN_H_