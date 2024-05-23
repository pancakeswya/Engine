#include "vk/device.h"
#include "vk/common.h"
#include "vk/exception.h"
#include "vk/swap_chain.h"
#include "vk/queue.h"

#include <array>
#include <set>
#include <vector>
#include <iostream>

namespace vk {

namespace {

constexpr std::array kExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

VkPhysicalDevice FindPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

  if (device_count == 0) {
    THROW_UNEXPECTED("failed to find GPUs with Vulkan support");
  }

  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

  for (const auto& device : devices) {
    if (auto [suitable, _] = queue::FindFamilyIndices(device, surface);
        suitable) {
      return device;
        }
  }
  THROW_UNEXPECTED("failed to find a suitable GPU");
}

} // namespace

bool Devices::ExtensionSupport(VkPhysicalDevice device) {
  uint32_t extension_count;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

  std::vector<VkExtensionProperties> available_extensions(extension_count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

  std::set<std::string> required_extensions(kExtensions.begin(), kExtensions.end());

  for (const auto& extension : available_extensions) {
    required_extensions.erase(extension.extensionName);
  }

  return required_extensions.empty();
}

Devices::Devices(VkInstance instance, VkSurfaceKHR surface)
    : physical_(FindPhysicalDevice(instance, surface)),
      logical_(), graphics_q_(), present_q_() {
  const auto[_, indices] = queue::FindFamilyIndices(physical_, surface);
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

  create_info.enabledExtensionCount = static_cast<uint32_t>(kExtensions.size());
  create_info.ppEnabledExtensionNames = kExtensions.data();

#ifdef DEBUG
  create_info.enabledLayerCount = static_cast<uint32_t>(common::kValidationLayers.size());
  create_info.ppEnabledLayerNames = common::kValidationLayers.data();
#else
  create_info.enabledLayerCount = 0;
#endif
  if (vkCreateDevice(physical_, &create_info, nullptr, &logical_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to create logical device");
  }
  vkGetDeviceQueue(logical_, indices.graphic, 0, &graphics_q_);
  vkGetDeviceQueue(logical_, indices.present, 0, &present_q_);
}

} // namespace vk