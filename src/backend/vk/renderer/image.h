#ifndef BACKEND_VK_RENDERER_IMAGE_H_
#define BACKEND_VK_RENDERER_IMAGE_H_

#include <vulkan/vulkan.h>

#include "backend/vk/renderer/handle.h"
#include "backend/vk/renderer/memory.h"

namespace vk {

class Image final : public DeviceHandle<VkImage> {
public:
  using DeviceHandle<VkImage>::DeviceHandle;

  [[nodiscard]] VkImageView view() const noexcept {
    return view_.handle();
  }

  [[nodiscard]] const Memory& memory() const noexcept {
    return memory_;
  }

  [[nodiscard]] VkFormat format() const noexcept {
    return format_;
  }

  [[nodiscard]] VkExtent2D extent() const noexcept {
    return extent_;
  }

  [[nodiscard]] uint32_t mip_levels() const noexcept {
    return mip_levels_;
  }
private:
  friend class Device;

  Memory memory_;
  DeviceHandle<VkImageView> view_;

  VkExtent2D extent_;
  VkFormat format_;
  uint32_t mip_levels_;

  explicit Image(
                 DeviceHandle<VkImage>&& image,
                 Memory&& memory,
                 DeviceHandle<VkImageView>&& view,
                 const VkExtent2D extent,
                 const VkFormat format,
                 const uint32_t mip_levels)
    : DeviceHandle<VkImage>(std::move(image)),
      memory_(std::move(memory)),
      view_(std::move(view)),
      extent_(extent),
      format_(format),
      mip_levels_(mip_levels) {}
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_IMAGE_H_
