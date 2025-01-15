#include "backend/vk/renderer/internal.h"

#include "backend/vk/renderer/error.h"
#include "backend/vk/renderer/image.h"

namespace vk::internal {

namespace {

int32_t FindMemoryType(const uint32_t type_filter, VkPhysicalDevice physical_device, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties mem_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

  for (int32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
    if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  throw Error("failed to find suitable memory type!");
}

} // namespace

Image CreateImage(VkDevice logical_device, VkPhysicalDevice physical_device, const VkAllocationCallbacks* allocator, const VkImageUsageFlags usage, const VkExtent2D extent, const VkFormat format, const VkImageTiling tiling, const uint32_t mip_levels) {
  VkImageCreateInfo image_info = {};
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.extent = { extent.width, extent.height, 1 };
  image_info.mipLevels = mip_levels;
  image_info.arrayLayers = 1;
  image_info.format = format;
  image_info.tiling = tiling;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage = usage;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkImage image = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateImage(logical_device, &image_info, allocator, &image); result != VK_SUCCESS) {
    throw Error("failed to create image!").WithCode(result);
  }
  return {
    image,
    logical_device,
    physical_device,
    allocator,
    extent,
    format,
    mip_levels
  };
}

Device::Dispatchable<VkImageView> CreateImageView(VkImage image, VkDevice logical_device, const VkAllocationCallbacks* allocator, const VkImageAspectFlags aspect_flags, const VkFormat format, const uint32_t mip_levels) {
  VkImageViewCreateInfo view_info = {};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image = image;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format = format;
  view_info.subresourceRange.aspectMask = aspect_flags;
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = mip_levels;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = 1;

  VkImageView image_view = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateImageView(logical_device, &view_info, allocator, &image_view); result != VK_SUCCESS) {
    throw Error("failed to create texture image view").WithCode(result);
  }
  return {
    image_view,
    logical_device,
    vkDestroyImageView,
    allocator
  };
}

Device::Dispatchable<VkDeviceMemory> CreateMemory(VkDevice logical_device, VkPhysicalDevice physical_device, const VkAllocationCallbacks* allocator, const VkMemoryPropertyFlags properties, const VkMemoryRequirements& mem_requirements) {
  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex = FindMemoryType(mem_requirements.memoryTypeBits, physical_device, properties);

  VkDeviceMemory buffer_memory = VK_NULL_HANDLE;
  if (const VkResult result = vkAllocateMemory(logical_device, &alloc_info, allocator, &buffer_memory); result != VK_SUCCESS) {
    throw Error("failed to allocate vertex buffer memory").WithCode(result);
  }
  return {
    buffer_memory,
    logical_device,
    vkFreeMemory,
    allocator
  };
}

} // namespace vk::internal
