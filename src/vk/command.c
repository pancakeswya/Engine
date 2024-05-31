#include "vk/command.h"
#include "vk/device.h"

#include <stdlib.h>

static Error cmdPoolCreate(VkDevice logical_device,
                           const VulkanQueueFamilyIndices* indices,
                           VkCommandPool* cmd_pool) {
  const VkCommandPoolCreateInfo create_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .queueFamilyIndex = indices->graphics,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT};
  const VkResult vk_res =
      vkCreateCommandPool(logical_device, &create_info, NULL, cmd_pool);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  return kSuccess;
}

static Error cmdBuffersCreate(VkDevice logical_device,
                              VkCommandPool cmd_pool,
                              const uint32_t count,
                              VkCommandBuffer** cmd_buffers_ptr,
                              uint32_t* cmd_buffer_count) {
  const VkCommandBufferAllocateInfo alloc_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = cmd_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = count};
  VkCommandBuffer* cmd_buffers =
      (VkCommandBuffer*)malloc(count * sizeof(VkCommandBuffer));
  const VkResult vk_res =
      vkAllocateCommandBuffers(logical_device, &alloc_info, cmd_buffers);
  if (vk_res != VK_SUCCESS) {
    free(cmd_buffers);
    return VulkanErrorCreate(vk_res);
  }
  *cmd_buffers_ptr = cmd_buffers;
  *cmd_buffer_count = count;

  return kSuccess;
}

Error VulkanCommandCreate(
  VkDevice logical_device,
  VulkanQueueFamilyIndices* indices,
  const uint32_t count,
  VulkanCommand* cmd) {
  Error err = cmdPoolCreate(logical_device, indices, &cmd->pool);
  if (!ErrorEqual(err, kSuccess)) {
    return err;
  }
  err = cmdBuffersCreate(logical_device, cmd->pool, count, &cmd->buffers, &cmd->buffer_count);
  if (!ErrorEqual(err, kSuccess)) {
    vkDestroyCommandPool(logical_device, cmd->pool, NULL);
    return err;
  }
  return kSuccess;
}

void VulkanCommandDestroy(VkDevice logical_device, VulkanCommand* cmd) {
  if (cmd->buffers != NULL) {
    vkFreeCommandBuffers(logical_device, cmd->pool, cmd->buffer_count, cmd->buffers);
    free(cmd->buffers);
  }
  if (cmd->pool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(logical_device, cmd->pool, NULL);
  }
}
