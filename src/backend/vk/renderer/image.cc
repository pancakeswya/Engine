#include "backend/vk/renderer/image.h"

#include "backend/vk/renderer/error.h"

namespace vk {

Image::Image() noexcept
  : mip_levels_(0), extent_(), format_() {}

Image::Image(Image&& other) noexcept
  : Base(std::move(other)),
    mip_levels_(other.mip_levels_),
    extent_(other.extent_),
    format_(other.format_),
    view_(std::move(other.view_)),
    memory_(std::move(other.memory_)) {
  other.mip_levels_ = 0;
  other.extent_ = {};
  other.format_ = {};
}

Image& Image::operator=(Image&& other) noexcept {
  if (this != &other) {
    Base::operator=(std::move(other));
    mip_levels_ = std::exchange(other.mip_levels_, 0);
    extent_ = std::exchange(other.extent_, {});
    format_ = std::exchange(other.format_, {});
    view_ = std::move(other.view_);
    memory_ = std::move(other.memory_);
  }
  return *this;
}

Image::Image(DeviceDispatchable<VkImage>&& image,
             DeviceDispatchable<VkImageView>&& view,
             DeviceDispatchable<VkDeviceMemory>&& memory,
             const VkExtent2D extent,
             const VkFormat format,
             const uint32_t mip_levels) noexcept
  : Base(std::move(image)),
    mip_levels_(mip_levels),
    extent_(extent),
    format_(format),
    view_(std::move(view)),
    memory_(std::move(memory)) {}

} // namespace vk
