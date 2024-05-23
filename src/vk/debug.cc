#include "vk/debug.h"
#include "vk/exception.h"
#include "vk/instance.h"

#include <iostream>
#include <string>

namespace vk::debug {

namespace {

VKAPI_ATTR VkBool32 VKAPI_CALL Callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
  std::cerr << pCallbackData->pMessage << std::endl;
  return VK_FALSE;
}

template<typename T>
void FunctionFromInstance(VkInstance instance, T& func, const std::string& name) {
  func = reinterpret_cast<T>(
      vkGetInstanceProcAddr(instance, name.c_str())
  );
  if (*func == nullptr) {
    THROW_UNEXPECTED("cant get" + name + "from instance");
  }
}

} // namespace

VkDebugUtilsMessengerCreateInfoEXT Messenger::CreateInfo() noexcept {
  VkDebugUtilsMessengerCreateInfoEXT info = {};
  info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  info.pfnUserCallback = Callback;
  return info;
}

Messenger::Messenger(Instance& instance)
  : instance_(instance.Get()), messenger_() {
  FunctionFromInstance(instance_, create_, "vkCreateDebugUtilsMessengerEXT");
  FunctionFromInstance(instance_, destroy_, "vkDestroyDebugUtilsMessengerEXT");
  if (const VkResult res = create_(instance_, &kCreateInfo, nullptr, &messenger_); res != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to set up debug messenger with code" + std::to_string(res));
  }
}

Messenger::~Messenger() {
  destroy_(instance_, messenger_, nullptr);
}

} // namespace vk