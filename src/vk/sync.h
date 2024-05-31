#ifndef VK_SYNC_H_
#define VK_SYNC_H_

#include "base/error.h"

#include <vulkan/vulkan.h>

typedef struct VulkanSyncObjects {
  VkSemaphore* image_semaphores;
  uint32_t image_semaphore_count;

  VkSemaphore* render_semaphores;
  uint32_t render_semaphore_count;

  VkFence* fences;
  uint32_t fence_count;
} VulkanSyncObjects;

extern Error VulkanSyncObjectsCreate(VkDevice logical_device, uint32_t count, VulkanSyncObjects* sync);
extern void VulkanSyncObjectDestroy(VkDevice logical_device, VulkanSyncObjects* sync);

#endif // VK_SYNC_H_