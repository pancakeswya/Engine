#include "backend/vk/renderer/instance.h"

#include <algorithm>
#include <cstring>
#include <vector>
#include <iostream>

#include "backend/vk/renderer/error.h"

namespace vk {

namespace {

VKAPI_ATTR VkBool32 VKAPI_CALL MessageCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT message_severity[[maybe_unused]],
  VkDebugUtilsMessageTypeFlagsEXT message_type[[maybe_unused]],
  const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
  void* user_data[[maybe_unused]])
{
  std::cerr << callback_data->pMessage << std::endl;
  return VK_FALSE;
}

bool InstanceLayersAreSupported(const std::vector<const char*>& layers) {
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (const char* layer : layers) {
    bool layer_found = false;
    for (const VkLayerProperties& layer_properties : available_layers) {
      if (std::strcmp(layer, layer_properties.layerName) == 0) {
        layer_found = true;
        break;
      }
    }
    if (!layer_found) {
      return false;
    }
  }
  return true;
}

bool LayersAreSupported(const std::vector<const char*>& layers) {
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (const char* layer : layers) {
    bool layer_found = false;
    for (const VkLayerProperties& layer_properties : available_layers) {
      if (std::strcmp(layer, layer_properties.layerName) == 0) {
        layer_found = true;
        break;
      }
    }
    if (!layer_found) {
      return false;
    }
  }
  return true;
}

VkDebugUtilsMessengerCreateInfoEXT GetMessengerCreateInfo() noexcept {
  VkDebugUtilsMessengerCreateInfoEXT create_info = {};

  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = MessageCallback;

  return create_info;
}

VkInstance CreateInstance(const std::vector<const char*>& extensions, const std::vector<const char*>& layers, const VkAllocationCallbacks* allocator) {
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "VulkanFun";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "Simple Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  if (!LayersAreSupported(layers)) {
    throw Error("Instance layers are not supported");
  }
  const VkDebugUtilsMessengerCreateInfoEXT messenger_info = GetMessengerCreateInfo();
  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();
#ifdef __APPLE__
  create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
  if (!layers.empty()) {
    create_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
    create_info.ppEnabledLayerNames = layers.data();
    create_info.pNext = &messenger_info;
  }
  VkInstance instance = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateInstance(&create_info, allocator, &instance); result != VK_SUCCESS) {
    throw Error("failed to create instance").WithCode(result);
  }
  return instance;
}

} // namespace

#define vkGetInstanceProcAddrByType(instance, proc) reinterpret_cast<decltype(&(proc))>(vkGetInstanceProcAddr(instance, #proc))

InstanceHandle<VkDebugUtilsMessengerEXT> Instance::CreateMessenger() const {
  VkInstance instance = this->handle();
  const VkAllocationCallbacks* allocator = this->allocator();

  const auto create_messenger = vkGetInstanceProcAddrByType(instance, vkCreateDebugUtilsMessengerEXT);
  if (create_messenger == nullptr) {
    throw Error("Couldn't find vkCreateDebugUtilsMessengerEXT by procc addr");
  }
  const auto destroy_messenger = vkGetInstanceProcAddrByType(instance, vkDestroyDebugUtilsMessengerEXT);
  if (destroy_messenger == nullptr) {
    throw Error("Couldn't find vkDestroyDebugUtilsMessengerEXT by procc addr");
  }
  const VkDebugUtilsMessengerCreateInfoEXT messenger_info = GetMessengerCreateInfo();

  VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
  if (const VkResult result = create_messenger(instance, &messenger_info, allocator, &messenger); result != VK_SUCCESS) {
    throw Error("failed to set up debug messenger").WithCode(result);
  }
  return {
    messenger,
    instance,
    destroy_messenger,
    allocator
  };
}

#undef vkGetInstanceProcAddrByType

Instance::Instance(const std::vector<const char*>& extensions, const std::vector<const char*>& layers, const VkAllocationCallbacks* allocator)
  : Handle(CreateInstance(extensions, layers, allocator), vkDestroyInstance, allocator) {}

std::vector<VkPhysicalDevice> Instance::EnumerateDevices() const {
  uint32_t device_count = 0;
  if (const VkResult result = vkEnumeratePhysicalDevices(handle(), &device_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get physical devices count").WithCode(result);
  }
  if (device_count == 0) {
    throw Error("failed to find GPUs with Vulkan support");
  }
  std::vector<VkPhysicalDevice> devices(device_count);
  if (const VkResult result = vkEnumeratePhysicalDevices(handle(), &device_count, devices.data()); result != VK_SUCCESS) {
    throw Error("failed to get physical devices").WithCode(result);
  }
  return devices;
}

InstanceHandle<VkSurfaceKHR> Instance::CreateSurface(const Window& window) const {
  VkInstance instance = this->handle();
  const VkAllocationCallbacks* allocator = this->allocator();

  VkSurfaceKHR surface = window.GetSurfaceFactory().CreateSurface(instance, allocator);
  return {
    surface,
    instance,
    vkDestroySurfaceKHR,
    allocator
  };
}

} // namespace vk