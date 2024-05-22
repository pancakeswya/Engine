#include "vk/debug.h"
#include "vk/exception.h"

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
    throw Exception("cant get" + name + "from instance");
  }
}

} // namespace

Messenger::CreateInfo::CreateInfo() : info_() {
  info_.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  info_.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  info_.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  info_.pfnUserCallback = Callback;
}

Messenger::Messenger(VkInstance instance)
  : messenger_() {
  instance_ = instance;
  const auto info = CreateInfo::Get();
  FunctionFromInstance(instance, create_, "vkCreateDebugUtilsMessengerEXT");
  FunctionFromInstance(instance, destroy_, "vkDestroyDebugUtilsMessengerEXT");
  if (VkResult res = create_(instance_, &info, nullptr, &messenger_); res != VK_SUCCESS) {
    throw Exception("failed to set up debug messenger with code" + std::to_string(res));
  }
}

Messenger::~Messenger() {
  destroy_(instance_, messenger_, nullptr);
}

} // namespace vk