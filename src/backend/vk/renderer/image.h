#ifndef BACKEND_VK_RENDERER_IMAGE_H_
#define BACKEND_VK_RENDERER_IMAGE_H_

#include "backend/vk/renderer/dispatchable.h"

#include <vulkan/vulkan.h>

namespace vk {

class Image : public DeviceDispatchable<VkImage> {
public:
  Image() noexcept;
  Image(const Image& other) = delete;
  Image(Image&& other) noexcept;
  ~Image() override = default;

  Image& operator=(const Image& other) = delete;
  Image& operator=(Image&& other) noexcept;

  [[nodiscard]] VkImageView GetView() const noexcept;
  [[nodiscard]] VkFormat GetFormat() const noexcept;
  [[nodiscard]] VkExtent2D GetExtent() const noexcept;
  [[nodiscard]] uint32_t GetMipLevels() const noexcept;
private:
  using Base = DeviceDispatchable<VkImage>;

  friend class DeviceDispatchableFactory;

  uint32_t mip_levels_;

  VkExtent2D extent_;
  VkFormat format_;

  DeviceDispatchable<VkImageView> view_;
  DeviceDispatchable<VkDeviceMemory> memory_;

  Image(DeviceDispatchable<VkImage>&& image,
        DeviceDispatchable<VkImageView>&& view,
        DeviceDispatchable<VkDeviceMemory>&& memory,
        VkExtent2D extent,
        VkFormat format,
        uint32_t mip_levels) noexcept;
};

inline VkImageView Image::GetView() const noexcept {
  return view_.GetHandle();
}

inline VkFormat Image::GetFormat() const noexcept {
  return format_;
}

inline VkExtent2D Image::GetExtent() const noexcept {
  return extent_;
}

inline uint32_t Image::GetMipLevels() const noexcept {
  return mip_levels_;
}

} // namespace vk

#endif // BACKEND_VK_RENDERER_IMAGE_H_
