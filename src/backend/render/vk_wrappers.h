#ifndef BACKEND_RENDER_VK_WRAPPERS_H_
#define BACKEND_RENDER_VK_WRAPPERS_H_

#include "backend/render/vk_types.h"
#include "backend/render/vk_factory.h"

#include <vulkan/vulkan.h>

namespace vk {

class Buffer {
public:
  DECL_UNIQUE_OBJECT(Buffer);

  Buffer(VkDevice logical_device, VkPhysicalDevice physical_device, VkBufferUsageFlags usage, uint32_t size)
    : size_(size),
      logical_device_(logical_device),
      physical_device_(physical_device),
      buffer_wrapper_(factory::CreateBuffer(logical_device, usage, size)) {}

  ~Buffer() = default;

  void Allocate(VkMemoryPropertyFlags properties) {
    memory_wrapper_ = factory::CreateBufferMemory(logical_device_, physical_device_, properties, buffer_wrapper_.get());
  }

  void Bind() const {
    if (const VkResult result = vkBindBufferMemory(logical_device_, buffer_wrapper_.get(), memory_wrapper_.get(), 0); result != VK_SUCCESS) {
      throw Error("failed to bind memory").WithCode(result);
    }
  }

  [[nodiscard]] VkBuffer Get() const noexcept {
    return buffer_wrapper_.get();
  }

  [[nodiscard]] uint32_t Size() const noexcept {
    return size_;
  }

  [[nodiscard]] void* Map() const {
    void* data;
    if (const VkResult result = vkMapMemory(logical_device_, memory_wrapper_.get(), 0, size_, 0, &data); result != VK_SUCCESS) {
      throw Error("failed to map buffer memory").WithCode(result);
    }
    return data;
  }

  void Unmap() const noexcept {
    vkUnmapMemory(logical_device_, memory_wrapper_.get());
  }
private:
  friend class BufferCommander;

  uint32_t size_;

  VkDevice logical_device_;
  VkPhysicalDevice physical_device_;

  HandleWrapper<VkBuffer> buffer_wrapper_;
  HandleWrapper<VkDeviceMemory> memory_wrapper_;
};

class Image {
public:
  DECL_UNIQUE_OBJECT(Image);

  Image(VkDevice logical_device,
             VkPhysicalDevice physical_device,
             const VkExtent2D extent,
             const uint32_t channels,
             const VkFormat format,
             const VkImageTiling tiling,
             const VkImageUsageFlags usage)
    : mip_levels_(CalculateMipMaps(extent)),
      channels_(channels),
      extent_(extent),
      format_(format),
      logical_device_(logical_device),
      physical_device_(physical_device),
      image_wrapper_(factory::CreateImage(logical_device, extent, format, tiling, usage, mip_levels_)) {}

  ~Image() = default;

  void Allocate(VkMemoryPropertyFlags properties) {
    memory_wrapper_ = factory::CreateImageMemory(logical_device_, physical_device_, properties, image_wrapper_.get());
  }

  void Bind() const {
    if (const VkResult result = vkBindImageMemory(logical_device_, image_wrapper_.get(), memory_wrapper_.get(), 0); result != VK_SUCCESS) {
      throw Error("failed to bind memory").WithCode(result);
    }
  }

  void CreateView(VkImageAspectFlags aspect_flags) {
    view_wrapper_ = factory::CreateImageView(logical_device_, image_wrapper_.get(), format_, aspect_flags, mip_levels_);
  }

  void CreateSampler(VkSamplerMipmapMode mipmap_mode) {
    sampler_wrapper_ = factory::CreateTextureSampler(logical_device_, physical_device_, mipmap_mode, mip_levels_);
  }

  [[nodiscard]] bool FormatFeatureSupported(VkFormatFeatureFlagBits feature) const {
    VkFormatProperties format_properties = {};
    vkGetPhysicalDeviceFormatProperties(physical_device_, format_, &format_properties);
    return (format_properties.optimalTilingFeatures & feature) != 0;
  }

  [[nodiscard]] VkImage Get() const noexcept {
    return image_wrapper_.get();
  }

  [[nodiscard]] VkImageView GetView() const noexcept {
    return view_wrapper_.get();
  }

  [[nodiscard]] VkSampler GetSampler() const noexcept {
    return sampler_wrapper_.get();
  }

  [[nodiscard]] VkFormat GetFormat() const noexcept {
    return format_;
  }
private:
  static uint32_t CalculateMipMaps(const VkExtent2D extent) {
    return static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1;
  }

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

} // namespace vk

#endif // BACKEND_RENDER_VK_WRAPPERS_H_