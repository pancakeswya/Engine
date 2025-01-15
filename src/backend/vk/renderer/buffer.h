#ifndef BACKEND_VK_RENDERER_BUFFER_H_
#define BACKEND_VK_RENDERER_BUFFER_H_

#include "backend/vk/renderer/device.h"

#include <vulkan/vulkan.h>

namespace vk {

class Buffer : public Device::Dispatchable<VkBuffer> {
public:
  Buffer() noexcept;
  Buffer(const Buffer& other) = delete;
  Buffer(Buffer&& other) noexcept;
  Buffer(VkBuffer buffer,
         VkDevice logical_device,
         VkPhysicalDevice physical_device,
         const VkAllocationCallbacks* allocator,
         uint32_t size) noexcept;
  ~Buffer() override = default;

  Buffer& operator=(const Buffer& other) = delete;
  Buffer& operator=(Buffer&& other) noexcept;

  void Bind() const;
  void Allocate(VkMemoryPropertyFlags properties);
  [[nodiscard]] void* Map() const;
  void Unmap() const noexcept;

  [[nodiscard]] uint32_t Size() const noexcept;
private:
  Dispatchable<VkDeviceMemory> memory_;
  VkPhysicalDevice physical_device_;

  uint32_t size_;
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_BUFFER_H_
