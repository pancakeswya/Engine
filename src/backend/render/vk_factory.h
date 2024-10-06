#ifndef BACKEND_RENDER_VK_FACTORY_H_
#define BACKEND_RENDER_VK_FACTORY_H_

#include "backend/render/vk_types.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <initializer_list>
#include <string>
#include <utility>

namespace vk::factory {

#ifdef DEBUG
extern HandleWrapper<VkDebugUtilsMessengerEXT> CreateMessenger(VkInstance instance);
#endif // DEBUG
extern HandleWrapper<VkInstance> CreateInstance();
extern HandleWrapper<VkSurfaceKHR> CreateSurface(VkInstance instance, GLFWwindow* window);
extern HandleWrapper<VkDevice> CreateLogicalDevice(VkPhysicalDevice physical_device, const QueueFamilyIndices& indices);
extern HandleWrapper<VkShaderModule> CreateShaderModule(VkDevice logical_device, const std::string& path);
extern HandleWrapper<VkRenderPass> CreateRenderPass(VkDevice logical_device, VkFormat image_format, VkFormat depth_format);
extern HandleWrapper<VkPipelineLayout> CreatePipelineLayout(VkDevice logical_device, VkDescriptorSetLayout descriptor_set_layout);
extern HandleWrapper<VkPipeline> CreatePipeline(VkDevice logical_device, VkPipelineLayout pipeline_layout, VkRenderPass render_pass, const std::initializer_list<ShaderStage>& shader_stages);
extern HandleWrapper<VkCommandPool> CreateCommandPool(VkDevice logical_device, QueueFamilyIndices indices);
extern HandleWrapper<VkSemaphore> CreateSemaphore(VkDevice logical_device);
extern HandleWrapper<VkFence> CreateFence(VkDevice logical_device);
extern HandleWrapper<VkBuffer> CreateBuffer(VkDevice logical_device, VkBufferUsageFlags usage, uint32_t data_size);
extern HandleWrapper<VkDeviceMemory> CreateMemory(VkDevice logical_device, VkPhysicalDevice physical_device, VkMemoryPropertyFlags properties, VkMemoryRequirements mem_requirements);
extern HandleWrapper<VkDeviceMemory> CreateImageMemory(VkDevice logical_device, VkPhysicalDevice physical_device, VkMemoryPropertyFlags properties, VkImage image);
extern HandleWrapper<VkDeviceMemory> CreateBufferMemory(VkDevice logical_device, VkPhysicalDevice physical_device, VkMemoryPropertyFlags properties, VkBuffer buffer);
extern HandleWrapper<VkDescriptorSetLayout> CreateDescriptorSetLayout(VkDevice logical_device);
extern HandleWrapper<VkDescriptorPool> CreateDescriptorPool(VkDevice logical_device, size_t count);
extern HandleWrapper<VkImage> CreateImage(VkDevice logical_device, VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, uint32_t mip_levels = 1);
extern HandleWrapper<VkImageView> CreateImageView(VkDevice logical_device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels = 1);
extern HandleWrapper<VkSampler> CreateTextureSampler(VkDevice logical_device, VkPhysicalDevice physical_device, uint32_t mip_levels);

extern std::pair<VkPhysicalDevice, QueueFamilyIndices> CreatePhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
extern std::pair<HandleWrapper<VkSwapchainKHR>, SwapchainDetails> CreateSwapchain(GLFWwindow* window, VkSurfaceKHR surface, VkPhysicalDevice physical_device, QueueFamilyIndices indices, VkDevice logical_device);

extern std::vector<VkImage> CreateSwapchainImages(VkSwapchainKHR swapchain, VkDevice logical_device);
extern std::vector<HandleWrapper<VkImageView>> CreateImageViews(const std::vector<VkImage>& images, VkDevice logical_device, VkFormat format);
extern std::vector<HandleWrapper<VkFramebuffer>> CreateFramebuffers(VkDevice logical_device, const std::vector<HandleWrapper<VkImageView>>& image_views, VkImageView depth_view, VkRenderPass render_pass,VkExtent2D extent);
extern std::vector<VkCommandBuffer> CreateCommandBuffers(VkDevice logical_device, VkCommandPool cmd_pool, uint32_t count);
extern std::vector<VkDescriptorSet> CreateDescriptorSets(VkDevice logical_device, VkDescriptorSetLayout descriptor_set_layout, VkDescriptorPool descriptor_pool, size_t count);

} // namespace vk::factory

#endif // BACKEND_RENDER_VK_FACTORY_H_