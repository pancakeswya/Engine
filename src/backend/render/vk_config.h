#ifndef BACKEND_RENDER_VK_CONFIG_H_
#define BACKEND_RENDER_VK_CONFIG_H_

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>

#define vkGetInstanceProcAddrByType(instance, proc) reinterpret_cast<decltype(&(proc))>(vkGetInstanceProcAddr(instance, #proc))

namespace vk {

struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;

  static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
  static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
};

namespace config {

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

} // namespace config

} // namesapce vk

#endif // BACKEND_RENDER_VK_CONFIG_H_