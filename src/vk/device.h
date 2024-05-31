#ifndef VK_DEVICE_H_
#define VK_DEVICE_H_

#include "base/error.h"

#include <vulkan/vulkan.h>

typedef struct VulkanSurfaceSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  VkSurfaceFormatKHR* formats;
  uint32_t formats_count;
  VkPresentModeKHR* present_modes;
  uint32_t present_modes_count;
} VulkanSurfaceSupportDetails;

typedef struct VulkanQueueFamilyIndices {
  uint32_t graphics, present;
} VulkanQueueFamilyIndices;

typedef struct VulkanDeviceInfo {
  VulkanSurfaceSupportDetails support_details;
  VulkanQueueFamilyIndices indices;
} VulkanDeviceInfo;

typedef struct VulkanDevice {
  VkPhysicalDevice physical;
  VkDevice logical;
  VulkanDeviceInfo info;
  VkQueue present_queue;
  VkQueue graphics_queue;
} VulkanDevice;

extern Error VulkanDeviceCreate(
    VkInstance instance,
    VkSurfaceKHR surface,
    const char** layers,
    uint32_t layer_count,
    VulkanDevice* device
);

extern void VulkanDeviceDestroy(
  VulkanDevice* device
);

#endif // VK_DEVICE_H_