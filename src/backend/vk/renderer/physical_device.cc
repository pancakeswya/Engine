#include "backend/vk/renderer/physical_device.h"

#include <set>

#include "backend/vk/renderer/error.h"

namespace vk {

SurfaceSupportDetails PhysicalDevice::GetSurfaceSupportDetails(VkSurfaceKHR surface) const {
  SurfaceSupportDetails details;
  if (const VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface, &details.capabilities); result != VK_SUCCESS) {
    throw Error("failed to get physical device surface capabilities").WithCode(result);
  }
  uint32_t format_count;
  if (const VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface, &format_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get physical device surface formats count").WithCode(result);
  }

  if (format_count != 0) {
    details.formats.resize(format_count);
    if (const VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface, &format_count, details.formats.data()); result != VK_SUCCESS) {
      throw Error("failed to get physical device surface formats").WithCode(result);
    }
  }
  uint32_t present_mode_count;
  if (const VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface, &present_mode_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get physical device surface present modes count").WithCode(result);
  }

  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    if (const VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface, &present_mode_count, details.present_modes.data()); result != VK_SUCCESS) {
      throw Error("failed to get physical device surface present modes").WithCode(result);
    }
  }
  return details;
}

bool PhysicalDevice::CheckFormatFeatureSupported(const VkFormat format, const VkFormatFeatureFlagBits feature) const {
  VkFormatProperties format_properties = {};
  vkGetPhysicalDeviceFormatProperties(physical_device_, format, &format_properties);
  return (format_properties.optimalTilingFeatures & feature) != 0;
}

int32_t PhysicalDevice::FindMemoryType(const uint32_t type_filter, VkMemoryPropertyFlags properties) const {
  VkPhysicalDeviceMemoryProperties mem_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device_, &mem_properties);

  for (int32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
    if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  throw Error("failed to find suitable memory type!");
}

VkFormat PhysicalDevice::FindSupportedFormat(const std::vector<VkFormat>& formats, const VkImageTiling tiling, const VkFormatFeatureFlags features) const {
  for (const VkFormat format : formats) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physical_device_, format, &props);
    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    }
    if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw Error("failed to find supported format");
}

bool PhysicalDevice::CheckExtensionsSupport(const std::vector<const char*>& extensions) const {
  uint32_t extension_count;
  if (const VkResult result = vkEnumerateDeviceExtensionProperties(physical_device_, nullptr, &extension_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get device extension properties count").WithCode(result);
  }
  std::vector<VkExtensionProperties> available_extensions(extension_count);
  if (const VkResult result = vkEnumerateDeviceExtensionProperties(physical_device_, nullptr, &extension_count, available_extensions.data()); result != VK_SUCCESS) {
    throw Error("failed to get device extension properties").WithCode(result);
  }
  std::set<std::string> required_extensions(extensions.begin(), extensions.end());

  for (const VkExtensionProperties& extension : available_extensions) {
    required_extensions.erase(extension.extensionName);
  }

  return required_extensions.empty();
}

std::vector<VkQueueFamilyProperties> PhysicalDevice::GetQueueFamilyProperties() const {
  uint32_t families_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &families_count, nullptr);

  std::vector<VkQueueFamilyProperties> family_properties(families_count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &families_count, family_properties.data());

  return family_properties;
}

VkBool32 PhysicalDevice::CheckSurfaceSupported(VkSurfaceKHR surface, const uint32_t queue_family_idx) const {
  VkBool32 present_support = false;
  if (const VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, queue_family_idx, surface, &present_support); result != VK_SUCCESS) {
    throw Error("failed to get physical device surface support").WithCode(result);
  }
  return present_support;
}

VkPhysicalDeviceFeatures PhysicalDevice::GetFeatures() const {
  VkPhysicalDeviceFeatures device_features;
  vkGetPhysicalDeviceFeatures(physical_device_, &device_features);

  return device_features;
}

} // namespace vk