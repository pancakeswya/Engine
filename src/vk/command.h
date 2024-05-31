#ifndef VK_COMMAND_H_
#define VK_COMMAND_H_

#include "base/error.h"

#include <vulkan/vulkan.h>

typedef struct VulkanQueueFamilyIndices VulkanQueueFamilyIndices;

typedef struct VulkanCommand {
  VkCommandPool pool;
  VkCommandBuffer* buffers;
  uint32_t buffer_count;
} VulkanCommand;

extern Error VulkanCommandCreate(
  VkDevice logical_device,
  VulkanQueueFamilyIndices* indices,
  uint32_t count,
  VulkanCommand* cmd);
extern void VulkanCommandDestroy(VkDevice logical_device, VulkanCommand* cmd);

#endif // VK_COMMAND_H_