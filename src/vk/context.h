#ifndef VK_CONTEXT_H_
#define VK_CONTEXT_H_

#include "base/error.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct SurfaceSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  VkSurfaceFormatKHR* formats;
  uint32_t formats_count;
  VkPresentModeKHR* present_modes;
  uint32_t present_modes_count;
} SurfaceSupportDetails;

typedef struct QueueFamilyIndices {
  uint32_t graphics, present;
} QueueFamilyIndices;

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

  SurfaceSupportDetails support_details;
  QueueFamilyIndices indices;
} VulkanContext;

Error VulkanContextCreate(VulkanContext* context, GLFWwindow* window, uint32_t frames);
Error VulkanContextRecreateSwapchain(VulkanContext* context, GLFWwindow* window);
void VulkanContextDestroy(VulkanContext* context);

#endif  // VK_CONTEXT_H_