#ifndef BACKEND_VK_RENDERER_IMAGE_H_
#define BACKEND_VK_RENDERER_IMAGE_H_

#include <vulkan/vulkan.h>

#include "backend/vk/renderer/dispatchable.h"
#include "backend/vk/renderer/memory.h"

namespace vk {
class Image : public DeviceDispatchable<VkImage> {
public:
  using DeviceDispatchable::DeviceDispatchable;

  [[nodiscard]] VkImageView GetView() const noexcept {
    return view_.GetHandle();
  }

  [[nodiscard]] const Memory& GetMemory() const noexcept {
    return memory_;
  }

  [[nodiscard]] VkFormat GetFormat() const noexcept {
    return format_;
  }

  [[nodiscard]] VkExtent2D GetExtent() const noexcept {
    return extent_;
  }

  [[nodiscard]] uint32_t GetMipLevels() const noexcept {
    return mip_levels_;
  }
private:
  friend class Device;

  Memory memory_;
  DeviceDispatchable<VkImageView> view_;

  VkExtent2D extent_;
  VkFormat format_;
  uint32_t mip_levels_;

  explicit Image(DeviceDispatchable&& image,
                 const VkExtent2D extent,
                 const VkFormat format,
                 const uint32_t mip_levels)
    : DeviceDispatchable(std::move(image)),
      extent_(extent),
      format_(format),
      mip_levels_(mip_levels) {}
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_IMAGE_H_
