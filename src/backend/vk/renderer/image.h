#ifndef BACKEND_VK_RENDERER_IMAGE_H_
#define BACKEND_VK_RENDERER_IMAGE_H_

#include "backend/vk/renderer/device.h"

#include <memory>

#include <vulkan/vulkan.h>

namespace vk {

class Image : public Device::Dispatchable<VkImage> {
public:
  Image() noexcept;
  Image(const Image& other) = delete;
  Image(Image&& other) noexcept;
  Image(VkImage image,
        VkDevice logical_device,
        VkPhysicalDevice physical_device,
        const VkAllocationCallbacks* allocator,
        VkExtent2D extent,
        VkFormat format,
        uint32_t mip_levels) noexcept;
  ~Image() override = default;

  Image& operator=(const Image& other) = delete;
  Image& operator=(Image&& other) noexcept;

  void Bind() const;
  void Allocate(VkMemoryPropertyFlags properties);
  void CreateView(VkImageAspectFlags aspect_flags);
  void CreateSampler(VkSamplerMipmapMode mipmap_mode);
  [[nodiscard]] bool FormatFeatureSupported(VkFormatFeatureFlagBits feature) const;
  [[nodiscard]] const Dispatchable<VkImageView>& View() const noexcept;
  [[nodiscard]] const Dispatchable<VkSampler>& Sampler() const noexcept;
  [[nodiscard]] VkFormat Format() const noexcept;
  [[nodiscard]] VkExtent2D Extent() const noexcept;
  [[nodiscard]] uint32_t MipLevels() const noexcept;
private:
  uint32_t mip_levels_;

  VkPhysicalDevice physical_device_;
  VkExtent2D extent_;
  VkFormat format_;

  Dispatchable<VkImageView> view_;
  Dispatchable<VkDeviceMemory> memory_;

  std::unique_ptr<Dispatchable<VkSampler>> sampler_;
};

inline const Device::Dispatchable<VkImageView>& Image::View() const noexcept {
  return view_;
}

inline const Device::Dispatchable<VkSampler>& Image::Sampler() const noexcept {
  return *sampler_;
}

inline VkFormat Image::Format() const noexcept {
  return format_;
}

inline VkExtent2D Image::Extent() const noexcept {
  return extent_;
}

inline uint32_t Image::MipLevels() const noexcept {
  return mip_levels_;
}

} // namespace vk

#endif // BACKEND_VK_RENDERER_IMAGE_H_
