#include "backend/vk/renderer/image.h"

#include "backend/vk/renderer/error.h"
#include "backend/vk/renderer/internal.h"

namespace vk {

Image::Image() noexcept
  : mip_levels_(0), physical_device_(VK_NULL_HANDLE), extent_(), format_(), sampler_(nullptr) {}

Image::Image(Image&& other) noexcept
  : Dispatchable(std::move(other)),
    mip_levels_(other.mip_levels_),
    physical_device_(other.physical_device_),
    extent_(other.extent_),
    format_(other.format_),
    view_(std::move(other.view_)),
    memory_(std::move(other.memory_)),
    sampler_(std::move(other.sampler_)) {
  other.mip_levels_ = 0;
  other.physical_device_ = VK_NULL_HANDLE;
  other.extent_ = {};
  other.format_ = {};
}

Image& Image::operator=(Image&& other) noexcept {
  if (this != &other) {
    static_cast<Dispatchable&>(*this) = static_cast<Dispatchable>(std::move(other));
    mip_levels_ = std::exchange(other.mip_levels_, 0);
    physical_device_ = std::exchange(other.physical_device_, VK_NULL_HANDLE);
    extent_ = std::exchange(other.extent_, {});
    format_ = std::exchange(other.format_, {});
    view_ = std::move(other.view_);
    memory_ = std::move(other.memory_);
    sampler_ = std::move(other.sampler_);
  }
  return *this;
}

Image::Image(VkImage image,
             VkDevice logical_device,
             VkPhysicalDevice physical_device,
             const VkAllocationCallbacks* allocator,
             const VkExtent2D extent,
             const VkFormat format,
             const uint32_t mip_levels) noexcept
  : Dispatchable(image, logical_device, vkDestroyImage, allocator),
    mip_levels_(mip_levels),
    physical_device_(physical_device),
    extent_(extent),
    format_(format) {}

void Image::Allocate(const VkMemoryPropertyFlags properties) {
  VkMemoryRequirements mem_requirements;
  vkGetImageMemoryRequirements(parent_, handle_, &mem_requirements);

  memory_ = internal::CreateMemory(parent_, physical_device_, allocator_, properties, mem_requirements);
}

void Image::Bind() const {
  if (const VkResult result = vkBindImageMemory(parent_, handle_, memory_.Handle(), 0); result != VK_SUCCESS) {
    throw Error("failed to bind memory").WithCode(result);
  }
}

void Image::CreateView(const VkImageAspectFlags aspect_flags) {
  view_ = internal::CreateImageView(handle_, parent_, allocator_, aspect_flags, format_, mip_levels_);
}

void Image::CreateSampler(const VkSamplerMipmapMode mipmap_mode) {
  VkPhysicalDeviceProperties properties = {};
  vkGetPhysicalDeviceProperties(physical_device_, &properties);

  VkSamplerCreateInfo sampler_info = {};
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.magFilter = VK_FILTER_LINEAR;
  sampler_info.minFilter = VK_FILTER_LINEAR;
  sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.anisotropyEnable = VK_TRUE;
  sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
  sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sampler_info.unnormalizedCoordinates = VK_FALSE;
  sampler_info.compareEnable = VK_FALSE;
  sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
  sampler_info.mipmapMode = mipmap_mode;
  sampler_info.minLod = 0.0f;
  sampler_info.maxLod = static_cast<float>(mip_levels_);
  sampler_info.mipLodBias = 0.0f;

  VkSampler sampler = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateSampler(parent_, &sampler_info, allocator_, &sampler); result != VK_SUCCESS) {
    throw Error("failed to create texture sampler").WithCode(result);
  }
  sampler_ = std::make_unique<Dispatchable<VkSampler>>(sampler, parent_, vkDestroySampler, allocator_);
}

bool Image::FormatFeatureSupported(const VkFormatFeatureFlagBits feature) const {
  VkFormatProperties format_properties = {};
  vkGetPhysicalDeviceFormatProperties(physical_device_, format_, &format_properties);
  return (format_properties.optimalTilingFeatures & feature) != 0;
}

} // namespace vk
