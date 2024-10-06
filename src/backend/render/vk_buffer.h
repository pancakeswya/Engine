#ifndef BACKEND_RENDER_VK_BUFFER_H_
#define BACKEND_RENDER_VK_BUFFER_H_

#include "backend/render/vk_types.h"

#include <vulkan/vulkan.h>

namespace vk {

class Buffer {
public:
  DECL_UNIQUE_OBJECT(Buffer);

  Buffer(VkDevice logical_device, VkPhysicalDevice device, VkBufferUsageFlags usage, uint32_t size);

  void Allocate(VkMemoryPropertyFlags properties);
  void Bind();

  [[nodiscard]] VkBuffer Get() const noexcept;
  [[nodiscard]] uint32_t Size() const noexcept;

  void* Map();
  void Unmap() noexcept;
private:
  friend class BufferCommander;

  uint32_t size_;

  VkDevice logical_device_;
  VkPhysicalDevice physical_device_;

  HandleWrapper<VkBuffer> buffer_wrapper_;
  HandleWrapper<VkDeviceMemory> memory_wrapper_;
};

inline VkBuffer Buffer::Get() const noexcept {
  return buffer_wrapper_.get();
}

inline uint32_t Buffer::Size() const noexcept {
  return size_;
}

} // namespace vk

#endif // BACKEND_RENDER_VK_BUFFER_H_