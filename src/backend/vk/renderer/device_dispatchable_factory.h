#ifndef BACKEND_VK_RENDERER_DEVICE_DISPATCHABLE_FACTORY_H_
#define BACKEND_VK_RENDERER_DEVICE_DISPATCHABLE_FACTORY_H_

#include "backend/vk/renderer/dispatchable.h"
#include "backend/vk/renderer/device.h"
#include "backend/vk/renderer/buffer.h"
#include "backend/vk/renderer/image.h"
#include "backend/vk/renderer/shader.h"
#include "backend/vk/renderer/swapchain.h"

namespace vk {

class DeviceDispatchableFactory {
public:
  explicit DeviceDispatchableFactory(const Device& device) noexcept;

  [[nodiscard]] ShaderModule CreateShaderModule(const ShaderInfo& shader_info) const;
  [[nodiscard]] DeviceDispatchable<VkRenderPass> CreateRenderPass(VkFormat image_format, VkFormat depth_format) const;
  [[nodiscard]] DeviceDispatchable<VkPipelineLayout> CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts) const;
  [[nodiscard]] DeviceDispatchable<VkPipeline> CreatePipeline(VkPipelineLayout pipeline_layout, VkRenderPass render_pass, const std::vector<VkVertexInputAttributeDescription>& attribute_descriptions, const std::vector<VkVertexInputBindingDescription>& binding_descriptions, const std::vector<ShaderModule>& shaders) const;
  [[nodiscard]] DeviceDispatchable<VkCommandPool> CreateCommandPool() const;
  [[nodiscard]] DeviceDispatchable<VkSemaphore> CreateSemaphore() const;
  [[nodiscard]] DeviceDispatchable<VkFence> CreateFence() const;
  [[nodiscard]] DeviceDispatchable<VkDescriptorSetLayout> CreateUboDescriptorSetLayout() const;
  [[nodiscard]] DeviceDispatchable<VkDescriptorSetLayout> CreateSamplerDescriptorSetLayout() const;
  [[nodiscard]] DeviceDispatchable<VkDescriptorPool> CreateDescriptorPool(size_t ubo_count, size_t texture_count) const;
  [[nodiscard]] DeviceDispatchable<VkDeviceMemory> CreateMemory(VkMemoryPropertyFlags properties, VkMemoryRequirements mem_requirements) const;
  [[nodiscard]] Buffer CreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t data_size) const;
  [[nodiscard]] Image CreateImage(VkImageUsageFlags usage,
                                  VkImageAspectFlags aspect_flags,
                                  VkMemoryPropertyFlags properties,
                                  VkExtent2D extent,
                                  VkFormat format,
                                  VkImageTiling tiling,
                                  uint32_t mip_levels = 1) const;
  [[nodiscard]] DeviceDispatchable<VkImageView> CreateImageView(VkImage image, VkImageAspectFlags aspect_flags, VkFormat format, uint32_t mip_levels = 1) const;
  [[nodiscard]] DeviceDispatchable<VkFramebuffer> CreateFramebuffer(const std::vector<VkImageView>& views, VkRenderPass render_pass, VkExtent2D extent) const;
  [[nodiscard]] DeviceDispatchable<VkSampler> CreateSampler(VkSamplerMipmapMode mipmap_mode, uint32_t mip_levels) const;
  [[nodiscard]] Swapchain CreateSwapchain(VkExtent2D size, VkSurfaceKHR surface) const;
  [[nodiscard]] std::vector<VkCommandBuffer> CreateCommandBuffers(VkCommandPool cmd_pool, uint32_t count) const;
private:
  const Device& device_;

  template<typename Handle, typename HandleInfo>
  using DeviceCreateFunc = VkResult(*)(VkDevice, const HandleInfo*, const VkAllocationCallbacks*, Handle*);

  template<typename Handle>
  using DeviceDestroyFunc = void(*)(VkDevice, Handle, const VkAllocationCallbacks*);

  template<typename Handle, typename HandleInfo>
  [[nodiscard]] DeviceDispatchable<Handle> ExecuteCreate(DeviceCreateFunc<Handle, HandleInfo> create_func, DeviceDestroyFunc<Handle> destroy_func, const HandleInfo* handle_info) const;
};

inline DeviceDispatchableFactory::DeviceDispatchableFactory(const Device& device) noexcept : device_(device) {}

} // namespace vk

#endif // BACKEND_VK_RENDERER_DEVICE_DISPATCHABLE_FACTORY_H_
