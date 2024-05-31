#include "vk/device.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static const char* kDeviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
static const uint32_t kDeviceExtensionsCount =
    sizeof(kDeviceExtensions) / sizeof(const char*);

static Error deviceExtensionsSupport(VkPhysicalDevice device,
                                     bool* support) {
  *support = true;
  uint32_t extension_count = 0;
  VkResult vk_res = vkEnumerateDeviceExtensionProperties(
      device, NULL, &extension_count, NULL);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  VkExtensionProperties* available_extensions = (VkExtensionProperties*)malloc(
      extension_count * sizeof(VkExtensionProperties));
  vk_res = vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count,
                                                available_extensions);
  if (vk_res != VK_SUCCESS) {
    free(available_extensions);
    return VulkanErrorCreate(vk_res);
  }
  for (uint32_t i = 0; i < kDeviceExtensionsCount; ++i) {
    bool found = false;
    for (uint32_t j = 0; j < extension_count; ++j) {
      if (strcmp(kDeviceExtensions[i], available_extensions[j].extensionName) ==
          0) {
        found = true;
        break;
      }
    }
    if (!found) {
      *support = false;
      break;
    }
  }
  free(available_extensions);
  return kSuccess;
}

static Error SurfaceSupport(VkPhysicalDevice device,
                            VkSurfaceKHR surface,
                            VulkanSurfaceSupportDetails* details) {
  VkSurfaceCapabilitiesKHR capabilities = {0};
  VkResult vk_res =
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  uint32_t formats_count = 0;
  vk_res = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formats_count,
                                                NULL);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  VkSurfaceFormatKHR* formats = NULL;
  if (formats_count != 0) {
    formats =
        (VkSurfaceFormatKHR*)malloc(formats_count * sizeof(VkSurfaceFormatKHR));
    if (formats == NULL) {
      return StdErrorCreate(kStdErrorOutOfMemory);
    }
    vk_res = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
                                                  &formats_count, formats);
    if (vk_res != VK_SUCCESS) {
      free(formats);
      return VulkanErrorCreate(vk_res);
    }
  }
  uint32_t present_modes_count = 0;
  vk_res = vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, surface, &present_modes_count, NULL);
  if (vk_res != VK_SUCCESS) {
    free(formats);
    return VulkanErrorCreate(vk_res);
  }
  VkPresentModeKHR* present_modes = NULL;
  if (present_modes_count != 0) {
    present_modes = (VkPresentModeKHR*)malloc(present_modes_count *
                                              sizeof(VkPresentModeKHR));
    if (present_modes == NULL) {
      free(formats);
      return StdErrorCreate(kStdErrorOutOfMemory);
    }
    vk_res = vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &present_modes_count, present_modes);
    if (vk_res != VK_SUCCESS) {
      free(present_modes);
      free(formats);
      return VulkanErrorCreate(vk_res);
    }
  }
  *details =
      (VulkanSurfaceSupportDetails){.capabilities = capabilities,
                                    .formats = formats,
                                    .formats_count = formats_count,
                                    .present_modes = present_modes,
                                    .present_modes_count = present_modes_count};
  return kSuccess;
}

static Error deviceIsSuitable(VkPhysicalDevice device,
                              VkSurfaceKHR surface,
                              bool* suitable,
                              VulkanDeviceInfo* info) {
  uint32_t graphics = 0, present = 0, families_count = 0;
  bool graphics_found = false, present_found = false;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &families_count, NULL);

  VkQueueFamilyProperties* families = (VkQueueFamilyProperties*)malloc(
      families_count * sizeof(VkQueueFamilyProperties));
  if (families == NULL) {
    return StdErrorCreate(kStdErrorOutOfMemory);
  }
  vkGetPhysicalDeviceQueueFamilyProperties(device, &families_count, families);

  for (uint32_t i = 0; i < families_count; ++i) {
    if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphics_found = true;
      graphics = i;
    }
    VkBool32 present_support = false;
    const VkResult vk_res = vkGetPhysicalDeviceSurfaceSupportKHR(
        device, i, surface, &present_support);
    if (vk_res != VK_SUCCESS) {
      free(families);
      return VulkanErrorCreate(vk_res);
    }
    if (present_support) {
      present_found = true;
      present = i;
    }
    if (graphics_found && present_found) {
      bool ext_support;
      Error err = deviceExtensionsSupport(device, &ext_support);
      if (!ErrorEqual(err, kSuccess)) {
        free(families);
        return err;
      }
      if (ext_support) {
        VulkanSurfaceSupportDetails support_details = {0};
        err = SurfaceSupport(device, surface, &support_details);
        if (!ErrorEqual(err, kSuccess)) {
          free(families);
          return err;
        }
        if (support_details.formats && support_details.present_modes) {
          *suitable = true;
          *info = (VulkanDeviceInfo) {
              .indices = {.present = present, .graphics = graphics},
              .support_details = support_details
          };
          break;
        }
      }
    }
  }
  free(families);
  return kSuccess;
}

static Error createPhysicalDevice(VkInstance instance,
                                  VkSurfaceKHR surface,
                                  VkPhysicalDevice* device,
                                  VulkanDeviceInfo* info) {
  uint32_t device_count = 0;
  VkResult vk_res = vkEnumeratePhysicalDevices(instance, &device_count, NULL);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  if (device_count == 0) {
    return AppErrorCreate(kAppErrorNoVulkanSupportedGpu);
  }
  VkPhysicalDevice* devices =
      (VkPhysicalDevice*)malloc(device_count * sizeof(VkPhysicalDevice));
  if (devices == NULL) {
    return StdErrorCreate(kStdErrorOutOfMemory);
  }
  vk_res = vkEnumeratePhysicalDevices(instance, &device_count, devices);
  if (vk_res != VK_SUCCESS) {
    free(devices);
    return VulkanErrorCreate(vk_res);
  }
  bool suitable = false;
  for (uint32_t i = 0; i < device_count; ++i) {
    const Error err = deviceIsSuitable(devices[i],
                                       surface,
                                       &suitable,
                                       info
    );
    if (!ErrorEqual(err, kSuccess)) {
      free(devices);
      return err;
    }
    if (suitable) {
      *device = devices[i];
      break;
    }
  }
  free(devices);
  return kSuccess;
}

static Error createLogicalDevice(VkPhysicalDevice physical_device,
                                 const VulkanQueueFamilyIndices* indices,
                                 const char** layers,
                                 const uint32_t layer_count,
                                 VkDevice* logical_device) {
  const uint32_t family_ids[] = {indices->graphics, indices->present};
  const size_t unique_family_count =
      (indices->graphics == indices->present) ? 1 : 2;
  VkDeviceQueueCreateInfo* queue_create_infos =
      (VkDeviceQueueCreateInfo*)malloc(unique_family_count *
                                       sizeof(VkDeviceQueueCreateInfo));
  if (queue_create_infos == NULL) {
    return StdErrorCreate(kStdErrorOutOfMemory);
  }
  const float queue_priority = 1.0f;
  for (uint32_t i = 0; i < unique_family_count; ++i) {
    const VkDeviceQueueCreateInfo queue_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = family_ids[i],
        .queueCount = 1,
        .pQueuePriorities = &queue_priority};
    queue_create_infos[i] = queue_create_info;
  }
  const VkPhysicalDeviceFeatures device_features = {0};

  const VkDeviceCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = unique_family_count,
      .pQueueCreateInfos = queue_create_infos,
      .pEnabledFeatures = &device_features,
      .enabledExtensionCount = kDeviceExtensionsCount,
      .ppEnabledExtensionNames = kDeviceExtensions,
#ifdef DEBUG
      .enabledLayerCount = layer_count,
      .ppEnabledLayerNames = layers,
#endif
  };
  const VkResult vk_res =
      vkCreateDevice(physical_device, &create_info, NULL, logical_device);
  free(queue_create_infos);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  return kSuccess;
}

static inline void destroyDeviceInfo(VulkanDeviceInfo* info) {
  free(info->support_details.formats);
  free(info->support_details.present_modes);
}

Error VulkanDeviceCreate(
    VkInstance instance,
    VkSurfaceKHR surface,
    const char** layers,
    const uint32_t layer_count,
    VulkanDevice* device
) {
  Error err = createPhysicalDevice(instance, surface, &device->physical, &device->info);
  if (!ErrorEqual(err, kSuccess)) {
    return err;
  }
  const VulkanQueueFamilyIndices indices = device->info.indices;
  err = createLogicalDevice(device->physical, &indices, layers, layer_count, &device->logical);
  if (!ErrorEqual(err, kSuccess)) {
    destroyDeviceInfo(&device->info);
    return err;
  }
  vkGetDeviceQueue(device->logical, indices.graphics, 0, &device->graphics_queue);
  vkGetDeviceQueue(device->logical, indices.present, 0, &device->present_queue);
  return kSuccess;
}

void VulkanDeviceDestroy(
    VulkanDevice* device
) {
  if (device != VK_NULL_HANDLE) {
    vkDestroyDevice(device->logical, NULL);
  }
  destroyDeviceInfo(&device->info);
}