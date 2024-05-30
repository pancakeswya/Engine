#ifndef VK_CONTEXT_H_
#define VK_CONTEXT_H_

#include "base/error.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct VulkanContext {
  VkInstance instance;
#ifdef DEBUG
  VkDebugUtilsMessengerEXT messenger;
#endif
  VkSurfaceKHR surface;

  VkPhysicalDevice physical_device;
  VkDevice logical_device;
  VkQueue present_queue;
  VkQueue graphics_queue;

  VkSwapchainKHR swapchain;
  VkExtent2D extent;
  VkFormat format;
  VkImage* images;
  uint32_t image_count;
  VkImageView* image_views;
  uint32_t image_view_count;

  VkRenderPass render_pass;
  VkPipeline pipeline;
  VkPipelineLayout pipeline_layout;
  VkFramebuffer* framebuffers;
  uint32_t framebuffer_count;

  VkCommandPool cmd_pool;
  VkCommandBuffer* cmd_buffers;
  uint32_t cmd_buffer_count;

  VkSemaphore* image_semaphores;
  uint32_t image_semaphore_count;
  VkSemaphore* render_semaphores;
  uint32_t render_semaphore_count;
  VkFence* fences;
  uint32_t fences_count;
} VulkanContext;

Error VulkanContextCreate(VulkanContext* context, GLFWwindow* window, uint32_t frames);
void VulkanContextDestroy(VulkanContext* context);

#endif  // VK_CONTEXT_H_