#include "backend/vk/renderer/device_selector.h"

#include <set>
#include <utility>

#include "backend/vk/renderer/error.h"
#include "backend/vk/renderer/instance.h"

namespace vk {

namespace {

struct QueueFamilyIndices {
  uint32_t graphic;
  uint32_t present;
};

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

SelfDispatchable<VkDevice> CreateDevice(VkPhysicalDevice physical_device, const QueueFamilyIndices& indices, const std::vector<const char*>& extensions, const VkAllocationCallbacks* allocator) {
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
  return {
    logical_device,
    vkDestroyDevice,
    allocator
  };
}

} // namespace

std::optional<Device> DeviceSelector::Select(const Requirements& requirements, const VkAllocationCallbacks* allocator) const {
  for(VkPhysicalDevice physical_device : physical_devices_) {
    if (auto[suitable, indices] = PhysicalDeviceIsSuitable(physical_device, requirements); suitable) {
      Device device = {};
      device.logical_device_ = CreateDevice(physical_device, indices, requirements.extensions, allocator);
      device.physical_device_ = physical_device;

      vkGetDeviceQueue(device.logical_device_.GetHandle(), indices.graphic, 0, &device.graphics_queue_.handle);
      device.graphics_queue_.family_index = indices.graphic;

      vkGetDeviceQueue(device.logical_device_.GetHandle(), indices.present, 0, &device.present_queue_.handle);
      device.present_queue_.family_index = indices.present;

      return device;
    }
  }
  return std::nullopt;
}

} // namespace vk