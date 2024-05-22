#include "vk/device.h"
#include "vk/common.h"
#include "vk/exception.h"

namespace vk::device {

namespace {

std::optional<uint32_t> FindGraphicFamilyIdx(VkPhysicalDevice device) {
  uint32_t families_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &families_count, nullptr);

  std::vector<VkQueueFamilyProperties> families(families_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &families_count, families.data());

  for (size_t i = 0; i < families_count; ++i) {
    if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      return static_cast<uint32_t>(i);
    }
  }
  return std::nullopt;
}

} // namespace

VkPhysicalDevice PhysicalFind(VkInstance instance) {
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

  if (device_count == 0) {
    THROW_UNEXPECTED("failed to find GPUs with Vulkan support");
  }

  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

  for (const auto& device : devices) {
    if (FindGraphicFamilyIdx(device).has_value()) {
      return device;
    }
  }
  THROW_UNEXPECTED("failed to find a suitable GPU");
}

Logical::Logical(VkInstance intance, VkPhysicalDevice physical_device) : device_(), graphics_q_() {
  const auto family_idx = FindGraphicFamilyIdx(physical_device);
  VkDeviceQueueCreateInfo queue_create_info = {};
  queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_create_info.queueFamilyIndex = family_idx.value();
  queue_create_info.queueCount = 1;

  constexpr float queuePriority = 1.0f;
  queue_create_info.pQueuePriorities = &queuePriority;

  constexpr VkPhysicalDeviceFeatures device_features = {};

  VkDeviceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  create_info.pQueueCreateInfos = &queue_create_info;
  create_info.queueCreateInfoCount = 1;

  create_info.pEnabledFeatures = &device_features;

  create_info.enabledExtensionCount = 0;

#ifdef DEBUG
  create_info.enabledLayerCount = static_cast<uint32_t>(common::kValidationLayers.size());
  create_info.ppEnabledLayerNames = common::kValidationLayers.data();
#endif
  create_info.enabledLayerCount = 0;
  if (vkCreateDevice(physical_device, &create_info, nullptr, &device_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to create logical device");
  }
  vkGetDeviceQueue(device_, family_idx.value(), 0, &graphics_q_);
}

} // namespace vk