#include "vk/instance.h"
#include "vk/layers.h"
#include "vk/exception.h"

#ifdef DEBUG
#include "vk/messenger.h"
#endif
#include <vector>
#include <iostream>

namespace vk {

Instance::Instance() : instance_() {
#ifdef DEBUG
  if (!layers::ValidationSupport()) {
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
  const auto extensions = layers::Extension();
  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();
#ifdef DEBUG
  const auto& messenger_create_info = Messenger::kCreateInfo;
  create_info.enabledLayerCount = static_cast<uint32_t>(layers::kValidation.size());
  create_info.ppEnabledLayerNames = layers::kValidation.data();
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