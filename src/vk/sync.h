#ifndef VK_SYNC_H_
#define VK_SYNC_H_

#include "vulkan/vulkan.h"

namespace vk::sync {

class Semaphore {
public:
  Semaphore(VkDevice logical_device);
  ~Semaphore();

  VkSemaphore get() noexcept;
protected:
  VkDevice logical_device_;
  VkSemaphore semaphore_;
};

inline VkSemaphore Semaphore::get() noexcept {
  return semaphore_;
}

class ImageSemaphore : public Semaphore {
public:
  ImageSemaphore(VkDevice logical_device);
  ~ImageSemaphore() = default;
  uint32_t AcquireNextImage(VkSwapchainKHR swapchain);
};

inline ImageSemaphore::ImageSemaphore(VkDevice logical_device) : Semaphore(logical_device) {}

inline uint32_t ImageSemaphore::AcquireNextImage(VkSwapchainKHR swapchain) {
  uint32_t image_idx;
  vkAcquireNextImageKHR(logical_device_, swapchain, UINT64_MAX, semaphore_, VK_NULL_HANDLE, &image_idx);
  return image_idx;
}

class Fence {
public:
  Fence(VkDevice logical_device);
  ~Fence();

  VkFence get() noexcept;

  void Wait() noexcept;
  void Reset() noexcept;
private:
  VkDevice logical_device_;
  VkFence fence_;
};

inline VkFence Fence::get() noexcept {
  return fence_;
}

inline void Fence::Wait() noexcept {
  vkWaitForFences(logical_device_, 1, &fence_, VK_TRUE, UINT64_MAX);
}

inline void Fence::Reset() noexcept {
  vkResetFences(logical_device_, 1, &fence_);
}



} // namespace vk::sync

#endif // VK_SYNC_H_