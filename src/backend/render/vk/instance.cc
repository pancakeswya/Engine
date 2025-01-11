#include "backend/render/vk/instance.h"

#include <vector>
#ifdef DEBUG
#include <cstring>
#include <iostream>
#endif

#include "backend/render/vk/error.h"
#include "backend/window/window.h"

namespace render::vk {

namespace {

#ifdef DEBUG

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


VKAPI_ATTR VkBool32 VKAPI_CALL MessageCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT message_severity[[maybe_unused]],
  VkDebugUtilsMessageTypeFlagsEXT message_type[[maybe_unused]],
  const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
  void* user_data[[maybe_unused]])
{
  std::cerr << callback_data->pMessage << std::endl;
  return VK_FALSE;
}

bool InstanceLayersIsSupported() {
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (const char* validation_layer : Instance::GetLayers()) {
    bool layer_found = false;
    for (const VkLayerProperties& layer_properties : available_layers) {
      if (std::strcmp(validation_layer, layer_properties.layerName) == 0) {
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

#endif

} // namespace

std::vector<const char*> Instance::GetLayers() {
  return {
    "VK_LAYER_KHRONOS_validation"
  };
}

Instance::Instance(const VkApplicationInfo& app_info, const std::vector<const char*>& extensions, const VkAllocationCallbacks* allocator)
  : handle_(VK_NULL_HANDLE), allocator_(allocator) {
#ifdef DEBUG
  const std::vector<const char*> layers = GetLayers();
  if (!InstanceLayersAreSupported(layers)) {
    throw Error("Instance layers are not supported");
  }
  const VkDebugUtilsMessengerCreateInfoEXT messenger_info = GetMessengerCreateInfo();
#endif  // DEBUG
  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();
#ifdef __APPLE__
  create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
#ifdef DEBUG
  create_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
  create_info.ppEnabledLayerNames = layers.data();
  create_info.pNext = &messenger_info;
#endif // DEBUG
  if (const VkResult result = vkCreateInstance(&create_info, allocator, &handle_); result != VK_SUCCESS) {
    throw Error("failed to create instance").WithCode(result);
  }
}

Instance::~Instance() {
  vkDestroyInstance(handle_, allocator_);
}

#ifdef DEBUG

#define vkGetInstanceProcAddrByType(instance, proc) reinterpret_cast<decltype(&(proc))>(vkGetInstanceProcAddr(instance, #proc))

Instance::Dispatchable<VkDebugUtilsMessengerEXT> Instance::CreateMessenger() const {
  const auto create_messenger = vkGetInstanceProcAddrByType(handle_, vkCreateDebugUtilsMessengerEXT);
  if (create_messenger == nullptr) {
    throw Error("Couldn't find vkCreateDebugUtilsMessengerEXT by procc addr");
  }
  const auto destroy_messenger = vkGetInstanceProcAddrByType(handle_, vkDestroyDebugUtilsMessengerEXT);
  if (destroy_messenger == nullptr) {
    throw Error("Couldn't find vkDestroyDebugUtilsMessengerEXT by procc addr");
  }
  const VkDebugUtilsMessengerCreateInfoEXT messenger_info = GetMessengerCreateInfo();

  VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
  if (const VkResult result = create_messenger(handle_, &messenger_info, allocator_, &messenger); result != VK_SUCCESS) {
    throw Error("failed to set up debug messenger").WithCode(result);
  }
  return {
    messenger,
    handle_,
    destroy_messenger,
    allocator_
  };
}

#undef vkGetInstanceProcAddrByType

#endif

Instance::Dispatchable<VkSurfaceKHR> Instance::CreateSurface(const window::vk::SurfaceFactory& surface_factory) const {
  VkSurfaceKHR surface = surface_factory.CreateSurface(handle_, allocator_);
  return {
    surface,
    handle_,
    vkDestroySurfaceKHR,
    allocator_
  };
}

std::vector<VkPhysicalDevice> Instance::EnumeratePhysicalDevices() const {
  uint32_t device_count = 0;
  if (const VkResult result = vkEnumeratePhysicalDevices(handle_, &device_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get physical devices count").WithCode(result);
  }
  if (device_count == 0) {
    throw Error("failed to find GPUs with Vulkan support");
  }
  std::vector<VkPhysicalDevice> devices(device_count);
  if (const VkResult result = vkEnumeratePhysicalDevices(handle_, &device_count, devices.data()); result != VK_SUCCESS) {
    throw Error("failed to get physical devices").WithCode(result);
  }
  return devices;
}

} // namespace vk