#ifndef BACKEND_VK_RENDERER_INTERNAL_H_
#define BACKEND_VK_RENDERER_INTERNAL_H_

#include "backend/vk/renderer/device.h"

#include <vulkan/vulkan.h>

namespace vk::internal {

extern Device::Dispatchable<VkImageView> CreateImageView(VkImage image, VkDevice logical_device, const VkAllocationCallbacks* allocator, VkImageAspectFlags aspect_flags, VkFormat format, uint32_t mip_levels = 1);
extern Image CreateImage(VkDevice logical_device, VkPhysicalDevice physical_device, const VkAllocationCallbacks* allocator, VkImageUsageFlags usage, VkExtent2D extent, VkFormat format, VkImageTiling tiling, uint32_t mip_levels = 1);
extern Device::Dispatchable<VkDeviceMemory> CreateMemory(VkDevice logical_device, VkPhysicalDevice physical_device, const VkAllocationCallbacks* allocator, VkMemoryPropertyFlags properties, const VkMemoryRequirements& mem_requirements);

} // namespace vk::internal

#endif // BACKEND_VK_RENDERER_INTERNAL_H_
