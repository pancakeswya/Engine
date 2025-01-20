#include "backend/vk/renderer/device.h"

#include <set>

#include "backend/vk/renderer/instance.h"
#include "backend/vk/renderer/error.h"

namespace vk {

namespace {

VkFormat FindSupportedFormat(const std::vector<VkFormat>& formats, VkPhysicalDevice physical_device, const VkImageTiling tiling, const VkFormatFeatureFlags features) {
  for (const VkFormat format : formats) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);
    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    }
    if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw Error("failed to find supported format");
}

VkDevice CreateDevice(VkPhysicalDevice physical_device, const QueueFamilyIndices& indices, const std::vector<const char*>& extensions, const VkAllocationCallbacks* allocator) {
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::set unique_family_ids = {
    indices.graphic,
    indices.present
  };
  constexpr float queue_priority = 1.0f;
  for(const unsigned int family_idx : unique_family_ids) {
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = family_idx;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(queue_create_info);
  }
#ifdef DEBUG
  const std::vector<const char*> layers = Instance::GetLayers();
#endif // DEBUG
  VkPhysicalDeviceFeatures device_features = {};
  device_features.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
  create_info.pQueueCreateInfos = queue_create_infos.data();
  create_info.pEnabledFeatures = &device_features;
  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();
#ifdef DEBUG
  create_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
  create_info.ppEnabledLayerNames = layers.data();
#endif // DEBUG
  create_info.enabledLayerCount = 0;

  VkDevice logical_device = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateDevice(physical_device, &create_info, allocator, &logical_device); result != VK_SUCCESS) {
    throw Error("failed to create logical device").WithCode(result);
  }
  return logical_device;
}

} // namespace

SurfaceSupportDetails Device::GetSurfaceSupport(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
  SurfaceSupportDetails details;
  if (const VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities); result != VK_SUCCESS) {
    throw Error("failed to get physical device surface capabilities").WithCode(result);
  }
  uint32_t format_count;
  if (const VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get physical device surface formats count").WithCode(result);
  }

  if (format_count != 0) {
    details.formats.resize(format_count);
    if (const VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, details.formats.data()); result != VK_SUCCESS) {
      throw Error("failed to get physical device surface formats").WithCode(result);
    }
  }
  uint32_t present_mode_count;
  if (const VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get physical device surface present modes count").WithCode(result);
  }

  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    if (const VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, details.present_modes.data()); result != VK_SUCCESS) {
      throw Error("failed to get physical device surface present modes").WithCode(result);
    }
  }
  return details;
}

Device::Device() noexcept : physical_device_(VK_NULL_HANDLE), queue_family_indices_() {}

Device::Device(VkPhysicalDevice physical_device, const QueueFamilyIndices& indices, const std::vector<const char*>& extensions, const VkAllocationCallbacks* allocator)
  : SelfDispatchable(CreateDevice(physical_device, indices, extensions, allocator), vkDestroyDevice, allocator),
    physical_device_(physical_device),
    queue_family_indices_(indices) {}

Device::Device(Device &&other) noexcept
    : SelfDispatchable(std::move(other)),
      physical_device_(other.physical_device_),
      queue_family_indices_(other.queue_family_indices_) {
  other.physical_device_ = VK_NULL_HANDLE;
  other.queue_family_indices_ = {};
}

Device& Device::operator=(Device&& other) noexcept {
  if (this != &other) {
    SelfDispatchable::operator=(std::move(other));
    physical_device_ = std::exchange(other.physical_device_, VK_NULL_HANDLE);
    queue_family_indices_ = std::exchange(other.queue_family_indices_, {});
  }
  return *this;
}

VkFormat Device::FindDepthFormat() const {
  return FindSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      physical_device_,
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
  );
}

bool Device::FormatFeatureSupported(const VkFormat format, const VkFormatFeatureFlagBits feature) const {
  VkFormatProperties format_properties = {};
  vkGetPhysicalDeviceFormatProperties(physical_device_, format, &format_properties);
  return (format_properties.optimalTilingFeatures & feature) != 0;
}

} // namespace vk