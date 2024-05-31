#include "vk/sync.h"

#include <stdlib.h>

static inline Error createSemaphore(VkDevice logical_device,
                             VkSemaphore* semaphore) {
  const VkSemaphoreCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  const VkResult vk_res =
      vkCreateSemaphore(logical_device, &create_info, NULL, semaphore);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  return kSuccess;
}

static inline Error createFence(VkDevice logical_device, VkFence* fence) {
  const VkFenceCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT};
  const VkResult vk_res =
      vkCreateFence(logical_device, &create_info, NULL, fence);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  return kSuccess;
}

static Error createSemaphores(
    VkDevice logical_device,
    const uint32_t count,
    VkSemaphore** semaphores_ptr,
    uint32_t* semaphore_count_ptr
) {
  VkSemaphore* semaphores = (VkSemaphore*)malloc(count * sizeof(VkSemaphore));
  if (semaphores == NULL) {
    return StdErrorCreate(kStdErrorOutOfMemory);
  }
  for(size_t i = 0; i < count; ++i) {
    const Error err = createSemaphore(logical_device, semaphores + i);
    if (!ErrorEqual(err, kSuccess)) {
      free(semaphores);
      return err;
    }
  }
  *semaphores_ptr = semaphores;
  *semaphore_count_ptr = count;

  return kSuccess;
}

static Error createFences(
    VkDevice logical_device,
    const uint32_t count,
    VkFence** fences_ptr,
    uint32_t* fence_count_ptr
) {
  VkFence* fences = (VkFence*)malloc(count * sizeof(VkFence));
  if (fences == NULL) {
    return StdErrorCreate(kStdErrorOutOfMemory);
  }
  for(size_t i = 0; i < count; ++i) {
    const Error err = createFence(logical_device, fences + i);
    if (!ErrorEqual(err, kSuccess)) {
      free(fences);
      return err;
    }
  }
  *fences_ptr = fences;
  *fence_count_ptr = count;

  return kSuccess;
}

static inline void destroySemaphores(VkDevice logical_device, VkSemaphore* semaphores, const uint32_t semaphore_count) {
  if (semaphores == NULL) {
    return;
  }
  for(size_t i = 0; i < semaphore_count; ++i) {
    vkDestroySemaphore(logical_device, semaphores[i], NULL);
  }
  free(semaphores);
}

static inline void destroyFences(VkDevice logical_device, VkFence* fences, const uint32_t fence_count) {
  if (fences == NULL) {
    return;
  }
  for(size_t i = 0; i < fence_count; ++i) {
    vkDestroyFence(logical_device, fences[i], NULL);
  }
  free(fences);
}

Error VulkanSyncObjectsCreate(VkDevice logical_device, const uint32_t count, VulkanSyncObjects* sync) {
  Error err = createSemaphores(logical_device, count, &sync->image_semaphores, &sync->image_semaphore_count);
  if (!ErrorEqual(err, kSuccess)) {
    return err;
  }
  err = createSemaphores(logical_device, count, &sync->render_semaphores, &sync->render_semaphore_count);
  if (!ErrorEqual(err, kSuccess)) {
    destroySemaphores(logical_device, sync->image_semaphores, sync->image_semaphore_count);
    return err;
  }
  err = createFences(logical_device, count, &sync->fences, &sync->fence_count);
  if (!ErrorEqual(err, kSuccess)) {
    destroySemaphores(logical_device, sync->image_semaphores, sync->image_semaphore_count);
    destroySemaphores(logical_device, sync->render_semaphores, sync->render_semaphore_count);
    return err;
  }
  return kSuccess;
}

void VulkanSyncObjectDestroy(VkDevice logical_device, VulkanSyncObjects* sync) {
  destroyFences(logical_device, sync->fences, sync->render_semaphore_count);
  destroySemaphores(logical_device, sync->render_semaphores, sync->render_semaphore_count);
  destroySemaphores(logical_device, sync->image_semaphores, sync->image_semaphore_count);
}