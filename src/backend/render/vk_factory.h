#ifndef BACKEND_RENDER_VK_FACTORY_H_
#define BACKEND_RENDER_VK_FACTORY_H_

#include "backend/render/vk_config.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>
#include <vector>
#include <memory>
#include <string>
#include <stdexcept>
#include <utility>
#include <type_traits>

namespace vk {

template<typename HandleType>
using HandleWrapper = std::unique_ptr<std::remove_pointer_t<HandleType>, std::function<void(HandleType)>>;

struct Error final : std::runtime_error {
  using runtime_error::runtime_error;

  [[nodiscard]] Error WithCode(const VkResult result) const {
    return Error{std::string(what()) + " [Code: " + std::to_string(result) + ']'};
  }
};

struct QueueFamilyIndices {
  uint32_t graphic, present;
};

struct PhysicalDeviceSurfaceDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

struct SwapchainDetails {
  VkExtent2D extent;
  VkFormat format;
};

namespace factory {

#ifdef DEBUG
extern HandleWrapper<VkDebugUtilsMessengerEXT> CreateMessenger(VkInstance instance);
#endif // DEBUG
extern HandleWrapper<VkInstance> CreateInstance();
extern HandleWrapper<VkSurfaceKHR> CreateSurface(VkInstance instance, GLFWwindow* window);
extern HandleWrapper<VkDevice> CreateLogicalDevice(VkPhysicalDevice physical_device, const QueueFamilyIndices& indices);
extern HandleWrapper<VkShaderModule> CreateShaderModule(VkDevice logical_device, const std::string& path);
extern HandleWrapper<VkRenderPass> CreateRenderPass(VkDevice logical_device, VkFormat format);
extern HandleWrapper<VkPipelineLayout> CreatePipelineLayout(VkDevice logical_device);
extern HandleWrapper<VkPipeline> CreatePipeline(VkDevice logical_device, VkPipelineLayout pipeline_layout, VkRenderPass render_pass, const std::vector<VkPipelineShaderStageCreateInfo> &shader_stages);
extern HandleWrapper<VkCommandPool> CreateCommandPool(VkDevice logical_device, QueueFamilyIndices indices);
extern HandleWrapper<VkSemaphore> CreateSemaphore(VkDevice logical_device);
extern HandleWrapper<VkFence> CreateFence(VkDevice logical_device);

extern std::pair<VkPhysicalDevice, QueueFamilyIndices> CreatePhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
extern std::pair<HandleWrapper<VkSwapchainKHR>, SwapchainDetails> CreateSwapchain(GLFWwindow* window, VkSurfaceKHR surface, VkPhysicalDevice physical_device, QueueFamilyIndices indices, VkDevice logical_device);

extern std::vector<VkImage> CreateSwapchainImages(VkSwapchainKHR swapchain, VkDevice logical_device);
extern std::vector<HandleWrapper<VkImageView>> CreateImageViews(const std::vector<VkImage>& images, VkDevice logical_device, VkFormat format);
extern std::vector<HandleWrapper<VkFramebuffer>> CreateFramebuffers(VkDevice logical_device, const std::vector<HandleWrapper<VkImageView>>& image_views, VkRenderPass render_pass,VkExtent2D extent);
extern std::vector<VkCommandBuffer> CreateCommandBuffers(VkDevice logical_device, VkCommandPool cmd_pool, uint32_t count);

} // namespace factory

} // namespace vk

#endif // BACKEND_RENDER_VK_FACTORY_H_