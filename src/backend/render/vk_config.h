#ifndef BACKEND_RENDER_VK_CONFIG_H_
#define BACKEND_RENDER_VK_CONFIG_H_

#include <vector>
#include <vulkan/vulkan.h>

#define vkGetInstanceProcAddrByType(instance, proc) reinterpret_cast<decltype(&(proc))>(vkGetInstanceProcAddr(instance, #proc))

namespace vk::config {

constexpr size_t kFrameCount = 2;

#ifdef DEBUG
extern bool InstanceLayersIsSupported();
extern std::vector<const char*> GetInstanceLayers();
#endif // DEBUG

extern VkApplicationInfo GetApplicationInfo() noexcept;
extern VkDebugUtilsMessengerCreateInfoEXT GetMessengerCreateInfo() noexcept;
extern const VkAllocationCallbacks* GetAllocationCallbacks() noexcept;

extern std::vector<const char*> GetInstanceExtensions();
extern std::vector<const char*> GetDeviceExtensions();
extern std::vector<VkDynamicState> GetDynamicStates();
extern std::vector<VkPipelineStageFlags> GetPipelineStageFlags();

} // namesapce vk::config

#endif // BACKEND_RENDER_VK_CONFIG_H_