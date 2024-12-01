#include "backend/render/vk/instance.h"

#include <vector>

#include "backend/render/vk/error.h"
#include "backend/render/vk/config.h"

namespace vk {

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

#endif

} // namespace

Instance::Instance(const VkAllocationCallbacks* allocator)
  : handle_(VK_NULL_HANDLE), allocator_(allocator) {
#ifdef DEBUG
  const std::vector<const char*> layers = config::GetInstanceLayers();
  if (!InstanceLayersAreSupported(layers)) {
    throw Error("Instance layers are not supported");
  }
  const VkDebugUtilsMessengerCreateInfoEXT messenger_info = config::GetMessengerCreateInfo();
#endif  // DEBUG
  const VkApplicationInfo app_info = config::GetApplicationInfo();

  const std::vector<const char*> extensions = config::GetInstanceExtensions();

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
Instance::Dispatchable<VkDebugUtilsMessengerEXT> Instance::CreateMessenger() const {
  const auto create_messenger = vkGetInstanceProcAddrByType(handle_, vkCreateDebugUtilsMessengerEXT);
  if (create_messenger == nullptr) {
    throw Error("Couldn't find vkCreateDebugUtilsMessengerEXT by procc addr");
  }
  const auto destroy_messenger = vkGetInstanceProcAddrByType(handle_, vkDestroyDebugUtilsMessengerEXT);
  if (destroy_messenger == nullptr) {
    throw Error("Couldn't find vkDestroyDebugUtilsMessengerEXT by procc addr");
  }
  const VkDebugUtilsMessengerCreateInfoEXT messenger_info = config::GetMessengerCreateInfo();

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
#endif

Instance::Dispatchable<VkSurfaceKHR> Instance::CreateSurface(GLFWwindow* window) const {
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  if (const VkResult result = glfwCreateWindowSurface(handle_, window, allocator_, &surface); result != VK_SUCCESS) {
    throw Error("failed to create window surface!").WithCode(result);
  }
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