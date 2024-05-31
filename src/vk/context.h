#ifndef VK_CONTEXT_H_
#define VK_CONTEXT_H_

#include "base/error.h"
#include "vk/command.h"
#include "vk/device.h"
#include "vk/render.h"
#include "vk/swapchain.h"
#include "vk/sync.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct VulkanContext {
  VkInstance instance;
#ifdef DEBUG
  VkDebugUtilsMessengerEXT messenger;
#endif
  VkSurfaceKHR surface;
  VulkanDevice device;
  VulkanSwapchain swapchain;
  VulkanSwapchainImages swap_images;
  VulkanRender render;
  VulkanCommand cmd;
  VulkanSync sync;
} VulkanContext;

extern Error VulkanContextCreate(VulkanContext* context, GLFWwindow* window, const uint32_t frames);
extern void VulkanContextDestroy(VulkanContext* context);

#endif  // VK_CONTEXT_H_