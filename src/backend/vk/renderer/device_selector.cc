#include "backend/vk/renderer/device_selector.h"

#include <set>
#include <utility>

#include "backend/vk/renderer/error.h"
#include "backend/vk/renderer/config.h"

namespace vk {

namespace {

bool PhysicalDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& extensions) {
  uint32_t extension_count;
  if (const VkResult result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get device extension properties count").WithCode(result);
  }
  std::vector<VkExtensionProperties> available_extensions(extension_count);
  if (const VkResult result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data()); result != VK_SUCCESS) {
    throw Error("failed to get device extension properties").WithCode(result);
  }
  std::set<std::string> required_extensions(extensions.begin(), extensions.end());

  for (const VkExtensionProperties& extension : available_extensions) {
    required_extensions.erase(extension.extensionName);
  }

  return required_extensions.empty();
}

std::pair<bool, QueueFamilyIndices> PhysicalDeviceIsSuitable(VkPhysicalDevice device, const DeviceSelector::Requirements& requirements) {
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
    if (const VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(device, i, requirements.surface, &present_support); result != VK_SUCCESS) {
      throw Error("failed to get physical device surface support").WithCode(result);
    }
    if (present_support) {
      present = static_cast<uint32_t>(i);
    }
    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(device, &supported_features);
    if ((!requirements.graphic || graphic.has_value()) &&
        (!requirements.present || present.has_value()) &&
        (!requirements.anisotropy || supported_features.samplerAnisotropy) &&
        PhysicalDeviceExtensionSupport(device, requirements.extensions)) {
      const SurfaceSupportDetails details = Device::GetSurfaceSupport(device, requirements.surface);
      if (!details.formats.empty() && !details.present_modes.empty()) {
        return {true, {graphic.value(), present.value()}};
      }
        }
  }
  return {};
}

} // namespace

std::optional<Device> DeviceSelector::Select(const Requirements& requirements) const {
  for(VkPhysicalDevice device : devices_) {
    if (auto[suitable, indices] = PhysicalDeviceIsSuitable(device, requirements); suitable) {
      return Device(device, indices, requirements.extensions);
    }
  }
  return std::nullopt;
}

} // namespace vk