#include "vk/instance.h"
#include "vk/common.h"
#include "vk/exception.h"

#ifdef DEBUG
#include "vk/debug.h"
#endif

#include <cstring>
#include <vector>

namespace vk {

namespace {

bool ValidationLayerSupport() {
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
  for (const char* validation_layer : common::kValidationLayers) {
    bool layer_found = false;
    for (const auto& layer_properties : available_layers) {
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

} // namespace

Instance::Instance() : instance_() {
#ifdef DEBUG
  if (!ValidationLayerSupport()) {
    THROW_UNEXPECTED("validation layers requested, but not available");
  }
#endif
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Hello Triangle";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  const auto extensions = common::ExtensionLayers();
  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();
#ifdef DEBUG
  const auto& messenger_create_info = debug::Messenger::kCreateInfo;
  create_info.enabledLayerCount = static_cast<uint32_t>(common::kValidationLayers.size());
  create_info.ppEnabledLayerNames = common::kValidationLayers.data();
  create_info.pNext = &messenger_create_info;
#endif
  if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS) {
    throw Exception("failed to create instance");
  }
}

Instance::~Instance() {
  vkDestroyInstance(instance_, nullptr);
}

} // namespace vk