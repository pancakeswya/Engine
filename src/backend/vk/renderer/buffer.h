#ifndef BACKEND_VK_RENDERER_BUFFER_H_
#define BACKEND_VK_RENDERER_BUFFER_H_

#include <vulkan/vulkan.h>

#include "backend/vk/renderer/dispatchable.h"

namespace vk {

class Buffer : public DeviceDispatchable<VkBuffer> {
public:
  Buffer() noexcept;
  Buffer(const Buffer& other) = delete;
  Buffer(Buffer&& other) noexcept;
  ~Buffer() override = default;

  Buffer& operator=(const Buffer& other) = delete;
  Buffer& operator=(Buffer&& other) noexcept;

  [[nodiscard]] void* Map() const;
  void Unmap() const noexcept;

  [[nodiscard]] uint32_t Size() const noexcept;
private:
  using Base = DeviceDispatchable<VkBuffer>;

  friend class DeviceDispatchableFactory;

  DeviceDispatchable<VkDeviceMemory> memory_;

  uint32_t size_;

  Buffer(DeviceDispatchable<VkBuffer>&& buffer,
         DeviceDispatchable<VkDeviceMemory>&& memory,
         uint32_t size) noexcept;
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_BUFFER_H_
