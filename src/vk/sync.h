#ifndef VK_SYNC_H_
#define VK_SYNC_H_

#include "base/error.h"

#include <vulkan/vulkan.h>

typedef struct VulkanSync {
  VkSemaphore* image_semaphores;
  uint32_t image_semaphore_count;

  VkSemaphore* render_semaphores;
  uint32_t render_semaphore_count;

  VkFence* fences;
  uint32_t fence_count;
} VulkanSync;

extern Error VulkanSyncCreate(VkDevice logical_device, uint32_t count, VulkanSync* sync);
extern void VulkanSyncDestroy(VkDevice logical_device, VulkanSync* sync);

#endif // VK_SYNC_H_