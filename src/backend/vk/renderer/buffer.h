#ifndef BACKEND_VK_RENDERER_BUFFER_H_
#define BACKEND_VK_RENDERER_BUFFER_H_

#include <vulkan/vulkan.h>

#include "backend/vk/renderer/dispatchable.h"
#include "backend/vk/renderer/memory.h"

namespace vk {

class Buffer : public DeviceDispatchable<VkBuffer> {
public:
  using DeviceDispatchable::DeviceDispatchable;

  [[nodiscard]] const Memory& GetMemory() const noexcept {
    return memory_;
  }

  [[nodiscard]] uint32_t Size() const noexcept {
    return size_;
  }
private:
  friend class Device;

  Memory memory_;
  uint32_t size_;

  explicit Buffer(DeviceDispatchable&& buffer, const uint32_t size) noexcept
    : DeviceDispatchable(std::move(buffer)), size_(size) {}
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_BUFFER_H_
