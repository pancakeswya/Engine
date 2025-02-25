#ifndef BACKEND_VK_RENDERER_DEVICE_H_
#define BACKEND_VK_RENDERER_DEVICE_H_

#include <vulkan/vulkan.h>

#include <vector>

#include "backend/vk/renderer/buffer.h"
#include "backend/vk/renderer/handle.h"
#include "backend/vk/renderer/image.h"
#include "backend/vk/renderer/physical_device.h"
#include "backend/vk/renderer/shader.h"
#include "backend/vk/renderer/swapchain.h"

namespace vk {

struct Queue {
  VkQueue handle;
  uint32_t family_index;
};

class Device final : public Handle<VkDevice> {
public:
  using Handle::Handle;

  [[nodiscard]] PhysicalDevice physical_device() const noexcept;

  [[nodiscard]] const Queue& graphics_queue() const noexcept;
  [[nodiscard]] const Queue& present_queue() const noexcept;

  [[nodiscard]] DeviceHandle<VkShaderModule> CreateShaderModule(const std::vector<uint32_t>& shader_info) const;
  [[nodiscard]] DeviceHandle<VkRenderPass> CreateRenderPass(VkFormat image_format, VkFormat depth_format) const;
  [[nodiscard]] DeviceHandle<VkPipelineLayout> CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts) const;
  [[nodiscard]] DeviceHandle<VkPipeline> CreatePipeline(VkPipelineLayout pipeline_layout, VkRenderPass render_pass, const std::vector<VkVertexInputAttributeDescription>& attribute_descriptions, const std::vector<VkVertexInputBindingDescription>& binding_descriptions, const std::vector<Shader>& shaders) const;
  [[nodiscard]] DeviceHandle<VkCommandPool> CreateCommandPool() const;
  [[nodiscard]] DeviceHandle<VkSemaphore> CreateSemaphore() const;
  [[nodiscard]] DeviceHandle<VkFence> CreateFence() const;
  [[nodiscard]] DeviceHandle<VkDescriptorSetLayout> CreateUniformDescriptorSetLayout() const;
  [[nodiscard]] DeviceHandle<VkDescriptorSetLayout> CreateSamplerDescriptorSetLayout() const;
  [[nodiscard]] DeviceHandle<VkDescriptorPool> CreateDescriptorPool(size_t uniform_count, size_t sampler_count) const;
  [[nodiscard]] DeviceHandle<VkImageView> CreateImageView(VkImage image, VkImageAspectFlags aspect_flags, VkFormat format, uint32_t mip_levels = 1) const;
  [[nodiscard]] DeviceHandle<VkFramebuffer> CreateFramebuffer(const std::vector<VkImageView>& views, VkRenderPass render_pass, VkExtent2D extent) const;
  [[nodiscard]] DeviceHandle<VkSampler> CreateSampler(VkSamplerMipmapMode mipmap_mode, uint32_t mip_levels) const;

  [[nodiscard]] std::vector<VkDescriptorSet> CreateDescriptorSets(VkDescriptorSetLayout descriptor_set_layout, VkDescriptorPool descriptor_pool, size_t count) const;
  [[nodiscard]] std::vector<VkCommandBuffer> CreateCommandBuffers(VkCommandPool cmd_pool, uint32_t count) const;

  [[nodiscard]] Memory CreateMemory(VkMemoryPropertyFlags properties, VkMemoryRequirements mem_requirements) const;
  [[nodiscard]] Buffer CreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t data_size) const;
  [[nodiscard]] Image CreateImage(VkImageUsageFlags usage,
                                  VkMemoryPropertyFlags properties,
                                  VkImageAspectFlags aspect_flags,
                                  VkExtent2D extent,
                                  VkFormat format,
                                  VkImageTiling tiling,
                                  uint32_t mip_levels = 1) const;
  [[nodiscard]] Swapchain CreateSwapchain(VkExtent2D size, VkSurfaceKHR surface) const;
private:
  friend class DeviceSelector;

  PhysicalDevice physical_device_;

  Queue graphics_queue_;
  Queue present_queue_;

  template<typename HandleType, typename HandleInfo>
  using DeviceCreateFunc = VkResult(*)(VkDevice, const HandleInfo*, const VkAllocationCallbacks*, HandleType*);

  template<typename HandleType>
  using DeviceDestroyFunc = void(*)(VkDevice, HandleType, const VkAllocationCallbacks*);

  template<typename HandleType, typename HandleInfo>
  using DeviceAllocateFunc = VkResult (*)(VkDevice, const HandleInfo*, HandleType*);

  template<typename HandleType, typename HandleInfo>
  [[nodiscard]] DeviceHandle<HandleType> ExecuteCreate(DeviceCreateFunc<HandleType, HandleInfo> create_func, DeviceDestroyFunc<HandleType> destroy_func, const HandleInfo* handle_info) const;

  template<typename Handle, typename HandleInfo>
  [[nodiscard]] std::vector<Handle> ExecuteAllocate(DeviceAllocateFunc<Handle, HandleInfo> allocate_func, uint32_t count, const HandleInfo* alloc_info) const;

  explicit Device(Handle&& device, PhysicalDevice physical_device, Queue graphics_queue, Queue present_queue) noexcept;
};

inline Device::Device(Handle&& device,
                      const PhysicalDevice physical_device,
                      const Queue graphics_queue,
                      const Queue present_queue) noexcept
  : Handle(std::move(device)),
    physical_device_(physical_device),
    graphics_queue_(graphics_queue),
    present_queue_(present_queue) {}

inline PhysicalDevice Device::physical_device() const noexcept {
  return physical_device_;
}

inline const Queue& Device::graphics_queue() const noexcept {
  return graphics_queue_;
}

inline const Queue& Device::present_queue() const noexcept {
  return present_queue_;
}

} // namespace vk

#endif // BACKEND_VK_RENDERER_DEVICE_H_