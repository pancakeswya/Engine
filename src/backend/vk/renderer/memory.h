#ifndef BACKEND_VK_RENDERER_MEMORY_H_
#define BACKEND_VK_RENDERER_MEMORY_H_

#include <vulkan/vulkan.h>

#include "backend/vk/renderer/dispatchable.h"
#include "backend/vk/renderer/error.h"

namespace vk {

class Memory : public DeviceDispatchable<VkDeviceMemory> {
public:
  using DeviceDispatchable::DeviceDispatchable;

  [[nodiscard]] void* Map(const VkDeviceSize size = VK_WHOLE_SIZE) const {
    void* data;
    if (const VkResult result = vkMapMemory(GetDevice(), GetHandle(), 0, size, 0, &data); result != VK_SUCCESS) {
      throw Error("failed to map buffer memory").WithCode(result);
    }
    return data;
  }

  void Unmap() const noexcept {
    vkUnmapMemory(GetDevice(), GetHandle());
  }
private:
  friend class Device;

  explicit Memory(DeviceDispatchable&& memory) noexcept : DeviceDispatchable(std::move(memory)) {}
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_MEMORY_H_
