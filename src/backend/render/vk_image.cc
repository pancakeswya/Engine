#include "backend/render/vk_image.h"
#include "backend/render/vk_factory.h"

#include <cmath>

namespace vk {

namespace {

inline uint32_t CalculateMipMaps(const VkExtent2D extent) {
  return static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1;
}

} // namespace

Image::Image(VkDevice logical_device,
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

void Image::Allocate(VkMemoryPropertyFlags properties) {
  memory_wrapper_ = factory::CreateImageMemory(logical_device_, physical_device_, properties, image_wrapper_.get());
}

void Image::Bind() {
  if (const VkResult result = vkBindImageMemory(logical_device_, image_wrapper_.get(), memory_wrapper_.get(), 0); result != VK_SUCCESS) {
    throw Error("failed to bind memory").WithCode(result);
  }
}

void Image::CreateView(VkImageAspectFlags aspect_flags) {
  view_wrapper_ = factory::CreateImageView(logical_device_, image_wrapper_.get(), format_, aspect_flags, mip_levels_);
}

void Image::CreateSampler(VkSamplerMipmapMode mipmap_mode) {
  sampler_wrapper_ = factory::CreateTextureSampler(logical_device_, physical_device_, mipmap_mode, mip_levels_);
}

bool Image::FormatFeatureSupported(VkFormatFeatureFlagBits feature) {
  VkFormatProperties format_properties = {};
  vkGetPhysicalDeviceFormatProperties(physical_device_, format_, &format_properties);
  return (format_properties.optimalTilingFeatures & feature) != 0;
}

} // namespace vk