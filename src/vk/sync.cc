#include "vk/sync.h"
#include "vk/exception.h"

namespace vk::sync {

Semaphore::Semaphore(VkDevice logical_device) : logical_device_(logical_device), semaphore_() {
  VkSemaphoreCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  if (vkCreateSemaphore(logical_device_, &create_info, nullptr, &semaphore_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to create semaphore");
  }
}

Semaphore::~Semaphore() {
  vkDestroySemaphore(logical_device_, semaphore_, nullptr);
}

Fence::Fence(VkDevice logical_device) : logical_device_(logical_device), fence_() {
  VkFenceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  if (vkCreateFence(logical_device, &create_info, nullptr, &fence_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to create semaphore");
  }
}

Fence::~Fence() {
  vkDestroyFence(logical_device_, fence_, nullptr);
}


} // namespace vk::sync