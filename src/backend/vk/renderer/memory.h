#ifndef BACKEND_VK_RENDERER_MEMORY_H_
#define BACKEND_VK_RENDERER_MEMORY_H_

#include <vulkan/vulkan.h>

#include "backend/vk/renderer/error.h"
#include "backend/vk/renderer/handle.h"

namespace vk {

class Memory final : public DeviceHandle<VkDeviceMemory> {
public:
  using DeviceHandle<VkDeviceMemory>::DeviceHandle;

  [[nodiscard]] void* Map(const VkDeviceSize size = VK_WHOLE_SIZE) const {
    void* data;
    if (const VkResult result = vkMapMemory(creator(), handle(), 0, size, 0, &data); result != VK_SUCCESS) {
      throw Error("failed to map buffer memory").WithCode(result);
    }
    return data;
  }

  void Unmap() const noexcept {
    vkUnmapMemory(creator(), handle());
  }
private:
  friend class Device;

  explicit Memory(DeviceHandle<VkDeviceMemory>&& memory) noexcept : DeviceHandle<VkDeviceMemory>(std::move(memory)) {}
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_MEMORY_H_
