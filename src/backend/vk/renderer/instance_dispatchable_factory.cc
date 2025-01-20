#include "backend/vk/renderer/instance_dispatchable_factory.h"

#include "backend/vk/renderer/error.h"

namespace vk {

#ifdef DEBUG

#define vkGetInstanceProcAddrByType(instance, proc) reinterpret_cast<decltype(&(proc))>(vkGetInstanceProcAddr(instance, #proc))

InstanceDispatchable<VkDebugUtilsMessengerEXT> InstanceDispatchableFactory::CreateMessenger() const {
  VkInstance instance = instance_.GetHandle();
  const VkAllocationCallbacks* allocator = instance_.GetAllocator();

  const auto create_messenger = vkGetInstanceProcAddrByType(instance, vkCreateDebugUtilsMessengerEXT);
  if (create_messenger == nullptr) {
    throw Error("Couldn't find vkCreateDebugUtilsMessengerEXT by procc addr");
  }
  const auto destroy_messenger = vkGetInstanceProcAddrByType(instance, vkDestroyDebugUtilsMessengerEXT);
  if (destroy_messenger == nullptr) {
    throw Error("Couldn't find vkDestroyDebugUtilsMessengerEXT by procc addr");
  }
  const VkDebugUtilsMessengerCreateInfoEXT messenger_info = Instance::GetMessengerCreateInfo();

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

#endif

InstanceDispatchable<VkSurfaceKHR> InstanceDispatchableFactory::CreateSurface(const Window& window) const {
  VkInstance instance = instance_.GetHandle();
  const VkAllocationCallbacks* allocator = instance_.GetAllocator();

  VkSurfaceKHR surface = window.GetSurfaceFactory().CreateSurface(instance, allocator);
  return {
    surface,
    instance,
    vkDestroySurfaceKHR,
    allocator
  };
}

} // namespace vk