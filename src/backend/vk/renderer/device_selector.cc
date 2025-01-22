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

std::pair<bool, QueueFamilyIndices> PhysicalDeviceIsSuitable(const PhysicalDevice& physical_device, const DeviceSelector::Requirements& requirements) {
  std::optional<uint32_t> graphic, present;

  std::vector<VkQueueFamilyProperties> queue_family_props = physical_device.GetQueueFamilyProperties();

  for (size_t i = 0; i < queue_family_props.size(); ++i) {
    if (queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphic = static_cast<uint32_t>(i);
    }
    if (physical_device.GetSurfaceSupported(requirements.surface, i)) {
      present = static_cast<uint32_t>(i);
    }
    if ((requirements.graphic && !graphic.has_value()) ||
        (requirements.present && !present.has_value())) {
      continue;
    }
    const VkPhysicalDeviceFeatures device_features = physical_device.GetPhysicalDeviceFeatures();
    if ((requirements.anisotropy && !device_features.samplerAnisotropy) ||
        !physical_device.GetExtensionsSupport(requirements.extensions)) {
      continue;
    }
    const PhysicalDevice::SurfaceSupportDetails details = physical_device.GetSurfaceSupportDetails(requirements.surface);
    if (!details.formats.empty() && !details.present_modes.empty()) {
      return {true, {graphic.value(), present.value()}};
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
  for(VkPhysicalDevice vk_physical_device : physical_devices_) {
    PhysicalDevice physical_device(vk_physical_device);
    if (auto[suitable, indices] = PhysicalDeviceIsSuitable(physical_device, requirements); suitable) {
      SelfDispatchable<VkDevice> device = CreateDevice(vk_physical_device, indices, requirements.extensions, allocator);

      Queue graphics_queue = {};
      vkGetDeviceQueue(device.GetHandle(), indices.graphic, 0, &graphics_queue.handle);
      graphics_queue.family_index = indices.graphic;

      Queue present_queue = {};
      vkGetDeviceQueue(device.GetHandle(), indices.present, 0, &present_queue.handle);
      present_queue.family_index = indices.present;

      return Device(
        std::move(device),
        physical_device,
        graphics_queue,
        present_queue
      );
    }
  }
  return std::nullopt;
}

} // namespace vk