#include "vk/device.h"
#include "vk/common.h"
#include "vk/exception.h"
#include "vk/instance.h"
#include "vk/surface.h"

#include <optional>
#include <utility>
#include <set>

#include <vector>

namespace vk {

namespace {

struct QueueFamilyIndices {
  uint32_t graphic, present;
};

std::pair<bool, QueueFamilyIndices> FindQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface) {
  std::optional<uint32_t> graphic, present;
  uint32_t families_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &families_count, nullptr);

  std::vector<VkQueueFamilyProperties> families(families_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &families_count, families.data());

  for (size_t i = 0; i < families_count; ++i) {
    if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphic = static_cast<uint32_t>(i);
    }
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
    if (present_support) {
      present = static_cast<uint32_t>(i);
    }
    if (graphic.has_value() && present.has_value()) {
      return {true, {graphic.value(), present.value()}};
    }
  }
  return {};
}

VkPhysicalDevice FindPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

  if (device_count == 0) {
    THROW_UNEXPECTED("failed to find GPUs with Vulkan support");
  }

  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

  for (const auto& device : devices) {
    if (auto[found, _] = FindQueueFamilyIndices(device, surface); found) {
      return device;
    }
  }
  THROW_UNEXPECTED("failed to find a suitable GPU");
}

} // namespace

Device::Device(Instance& instance, Surface& surface) : logical_device_(), graphics_q_(), present_q_() {
  physical_device_ = FindPhysicalDevice(instance.Get(), surface.Get());
  const auto[_, indices] = FindQueueFamilyIndices(physical_device_, surface.Get());
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::set unique_family_ids = {
    indices.graphic,
    indices.present
  };
  constexpr float queue_priority = 1.0f;
  for(const auto& family_idx : unique_family_ids) {
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = family_idx;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(queue_create_info);
  }

  constexpr VkPhysicalDeviceFeatures device_features = {};

  VkDeviceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
  create_info.pQueueCreateInfos = queue_create_infos.data();

  create_info.pEnabledFeatures = &device_features;

  create_info.enabledExtensionCount = 0;

#ifdef DEBUG
  create_info.enabledLayerCount = static_cast<uint32_t>(common::kValidationLayers.size());
  create_info.ppEnabledLayerNames = common::kValidationLayers.data();
#else
  create_info.enabledLayerCount = 0;
#endif
  if (vkCreateDevice(physical_device_, &create_info, nullptr, &logical_device_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to create logical device");
  }
  vkGetDeviceQueue(logical_device_, indices.graphic, 0, &graphics_q_);
  vkGetDeviceQueue(logical_device_, indices.present, 0, &present_q_);
}

} // namespace vk