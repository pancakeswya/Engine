#include "vk/device.h"
#include "vk/common.h"
#include "vk/exception.h"
#include "vk/swap_chain.h"

#include <optional>
#include <utility>
#include <set>

#include <vector>

namespace vk::device {

namespace physical {

namespace {

struct QueueFamilyIndices {
  uint32_t graphic, present;
};

std::pair<bool, QueueFamilyIndices> FindQueueFamilyIndices(
    VkPhysicalDevice device, VkSurfaceKHR surface) {
  std::optional<uint32_t> graphic, present;
  uint32_t families_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &families_count, nullptr);

  std::vector<VkQueueFamilyProperties> families(families_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &families_count,
                                           families.data());

  for (size_t i = 0; i < families_count; ++i) {
    if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphic = static_cast<uint32_t>(i);
    }
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
    if (present_support) {
      present = static_cast<uint32_t>(i);
    }
    if (graphic.has_value() &&
        present.has_value() &&
        physical::ExtensionSupport(device)) {
      auto details = physical::SwapChainSupport(device , surface);
      if (!details.formats.empty() && !details.present_modes.empty()) {
        return {true, {graphic.value(), present.value()}};
      }
    }
  }
  return {};
}

}  // namespace

VkPhysicalDevice Find(VkInstance instance, VkSurfaceKHR surface) {
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

  if (device_count == 0) {
    THROW_UNEXPECTED("failed to find GPUs with Vulkan support");
  }

  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

  for (const auto& device : devices) {
    if (auto [suitable, _] = FindQueueFamilyIndices(device, surface);
        suitable) {
      return device;
    }
  }
  THROW_UNEXPECTED("failed to find a suitable GPU");
}

bool ExtensionSupport(VkPhysicalDevice device) {
  uint32_t extension_count;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                       nullptr);

  std::vector<VkExtensionProperties> available_extensions(extension_count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                       available_extensions.data());

  std::set required_extensions(kExtensions.begin(), kExtensions.end());

  for (const auto& extension : available_extensions) {
    required_extensions.erase(extension.extensionName);
  }
  return required_extensions.empty();
}

SwapChainSupportDetails SwapChainSupport(VkPhysicalDevice device,
                                         VkSurfaceKHR surface) {
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);

  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count,
                                         details.formats.data());
  }

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                            &present_mode_count, nullptr);

  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &present_mode_count, details.present_modes.data());
  }
  return details;
}

} // namespace physical

Logical::Logical(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
    : device_(), graphics_q_(), present_q_() {
  const auto[_, indices] = physical::FindQueueFamilyIndices(physical_device, surface);
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
  if (vkCreateDevice(physical_device, &create_info, nullptr, &device_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to create logical device");
  }
  vkGetDeviceQueue(device_, indices.graphic, 0, &graphics_q_);
  vkGetDeviceQueue(device_, indices.present, 0, &present_q_);
}

} // namespace vk