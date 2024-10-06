#ifndef BACKEND_RENDER_VK_IMAGE_H_
#define BACKEND_RENDER_VK_IMAGE_H_

#include "backend/render/vk_types.h"

#include <vulkan/vulkan.h>

namespace vk {

class Image {
public:
  DECL_UNIQUE_OBJECT(Image);

  Image(VkDevice logical_device,
        VkPhysicalDevice physical_device,
        VkExtent2D extent,
        uint32_t channels,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage);

  void Allocate(VkMemoryPropertyFlags properties);
  void Bind();

  void CreateView(VkImageAspectFlags aspect_flags);
  void CreateSampler(VkSamplerMipmapMode mipmap_mode);

  bool FormatFeatureSupported(VkFormatFeatureFlagBits feature);

  [[nodiscard]] VkImage Get() const noexcept;
  [[nodiscard]] VkImageView GetView() const noexcept;
  [[nodiscard]] VkSampler GetSampler() const noexcept;
  [[nodiscard]] VkFormat GetFormat() const noexcept;
private:
  friend class ImageCommander;

  uint32_t mip_levels_;
  uint32_t channels_;

  VkExtent2D extent_;
  VkFormat format_;

  VkDevice logical_device_;
  VkPhysicalDevice physical_device_;

  HandleWrapper<VkImage> image_wrapper_;
  HandleWrapper<VkImageView> view_wrapper_;
  HandleWrapper<VkSampler> sampler_wrapper_;
  HandleWrapper<VkDeviceMemory> memory_wrapper_;
};

inline VkImage Image::Get() const noexcept {
  return image_wrapper_.get();
}

inline VkImageView Image::GetView() const noexcept {
  return view_wrapper_.get();
}

inline VkFormat Image::GetFormat() const noexcept {
  return format_;
}

inline VkSampler Image::GetSampler() const noexcept {
  return sampler_wrapper_.get();
}

} // namespace vk

#endif // BACKEND_RENDER_VK_IMAGE_H_