#ifndef BACKEND_VK_RENDERER_BUFFER_H_
#define BACKEND_VK_RENDERER_BUFFER_H_

#include <vulkan/vulkan.h>

#include "backend/vk/renderer/handle.h"
#include "backend/vk/renderer/memory.h"

namespace vk {

class Buffer final : public DeviceHandle<VkBuffer> {
public:
  using DeviceHandle<VkBuffer>::DeviceHandle;

  [[nodiscard]] const Memory& memory() const noexcept {
    return memory_;
  }

  [[nodiscard]] uint32_t size() const noexcept {
    return size_;
  }
private:
  friend class Device;

  Memory memory_;
  uint32_t size_;

  explicit Buffer(DeviceHandle<VkBuffer>&& buffer, Memory&& memory, const uint32_t size) noexcept
    : DeviceHandle<VkBuffer>(std::move(buffer)), memory_(std::move(memory)), size_(size) {}
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_BUFFER_H_
