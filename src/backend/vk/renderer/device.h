#ifndef BACKEND_VK_RENDERER_DEVICE_H_
#define BACKEND_VK_RENDERER_DEVICE_H_

#include <vector>

#include <vulkan/vulkan.h>

#include "backend/vk/renderer/dispatchable.h"
#include "backend/vk/renderer/buffer.h"
#include "backend/vk/renderer/image.h"
#include "backend/vk/renderer/swapchain.h"
#include "backend/vk/renderer/shader.h"
#include "backend/vk/renderer/physical_device.h"

namespace vk {

struct Queue {
  VkQueue handle;
  uint32_t family_index;
};

class Device final : public SelfDispatchable<VkDevice> {
public:
  using SelfDispatchable::SelfDispatchable;

  [[nodiscard]] PhysicalDevice GetPhysicalDevice() const noexcept;

  [[nodiscard]] const Queue& GetGraphicsQueue() const noexcept;
  [[nodiscard]] const Queue& GetPresentQueue() const noexcept;

  [[nodiscard]] DeviceDispatchable<VkShaderModule> CreateShaderModule(const std::vector<uint32_t>& shader_info) const;
  [[nodiscard]] DeviceDispatchable<VkRenderPass> CreateRenderPass(VkFormat image_format, VkFormat depth_format) const;
  [[nodiscard]] DeviceDispatchable<VkPipelineLayout> CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts) const;
  [[nodiscard]] DeviceDispatchable<VkPipeline> CreatePipeline(VkPipelineLayout pipeline_layout, VkRenderPass render_pass, const std::vector<VkVertexInputAttributeDescription>& attribute_descriptions, const std::vector<VkVertexInputBindingDescription>& binding_descriptions, const std::vector<Shader>& shaders) const;
  [[nodiscard]] DeviceDispatchable<VkCommandPool> CreateCommandPool(uint32_t family_index) const;
  [[nodiscard]] DeviceDispatchable<VkSemaphore> CreateSemaphore() const;
  [[nodiscard]] DeviceDispatchable<VkFence> CreateFence() const;
  [[nodiscard]] DeviceDispatchable<VkDescriptorSetLayout> CreateUniformDescriptorSetLayout() const;
  [[nodiscard]] DeviceDispatchable<VkDescriptorSetLayout> CreateSamplerDescriptorSetLayout() const;
  [[nodiscard]] DeviceDispatchable<VkDescriptorPool> CreateDescriptorPool(size_t uniform_count, size_t sampler_count) const;
  [[nodiscard]] DeviceDispatchable<VkImageView> CreateImageView(VkImage image, VkImageAspectFlags aspect_flags, VkFormat format, uint32_t mip_levels = 1) const;
  [[nodiscard]] DeviceDispatchable<VkFramebuffer> CreateFramebuffer(const std::vector<VkImageView>& views, VkRenderPass render_pass, VkExtent2D extent) const;
  [[nodiscard]] DeviceDispatchable<VkSampler> CreateSampler(VkSamplerMipmapMode mipmap_mode, uint32_t mip_levels) const;

  [[nodiscard]] std::vector<VkDescriptorSet> CreateDescriptorSets(VkDescriptorSetLayout descriptor_set_layout, VkDescriptorPool descriptor_pool, size_t count) const;
  [[nodiscard]] std::vector<VkCommandBuffer> CreateCommandBuffers(VkCommandPool cmd_pool, uint32_t count) const;

  [[nodiscard]] Memory CreateMemory(VkMemoryPropertyFlags properties, VkMemoryRequirements mem_requirements) const;

  [[nodiscard]] Buffer CreateBuffer(VkBufferUsageFlags usage, uint32_t data_size) const;
  void AllocateBuffer(Buffer& buffer, VkMemoryPropertyFlags properties) const;
  void BindBuffer(const Buffer& buffer) const;

  [[nodiscard]] Image CreateImage(VkImageUsageFlags usage,
                                  VkExtent2D extent,
                                  VkFormat format,
                                  VkImageTiling tiling,
                                  uint32_t mip_levels = 1) const;
  void AllocateImage(Image& image, VkMemoryPropertyFlags properties) const;
  void BindImage(const Image& image) const;
  void MakeImageView(Image& image, VkImageAspectFlags aspect_flags) const;

  [[nodiscard]] Swapchain CreateSwapchain(VkExtent2D size, VkSurfaceKHR surface) const;
private:
  friend class DeviceSelector;

  PhysicalDevice physical_device_;

  Queue graphics_queue_;
  Queue present_queue_;

  template<typename Handle, typename HandleInfo>
  using DeviceCreateFunc = VkResult(*)(VkDevice, const HandleInfo*, const VkAllocationCallbacks*, Handle*);

  template<typename Handle>
  using DeviceDestroyFunc = void(*)(VkDevice, Handle, const VkAllocationCallbacks*);

  template<typename Handle, typename HandleInfo>
  using DeviceAllocateFunc = VkResult (*)(VkDevice, const HandleInfo*, Handle*);

  template<typename Handle, typename HandleInfo>
  [[nodiscard]] DeviceDispatchable<Handle> ExecuteCreate(DeviceCreateFunc<Handle, HandleInfo> create_func, DeviceDestroyFunc<Handle> destroy_func, const HandleInfo* handle_info) const;

  template<typename Handle, typename HandleInfo>
  [[nodiscard]] std::vector<Handle> ExecuteAllocate(DeviceAllocateFunc<Handle, HandleInfo> allocate_func, uint32_t count, const HandleInfo* alloc_info) const;

  explicit Device(SelfDispatchable&& device, PhysicalDevice physical_device, Queue graphics_queue, Queue present_queue) noexcept;
};

inline Device::Device(SelfDispatchable&& device,
                      PhysicalDevice physical_device,
                      Queue graphics_queue,
                      Queue present_queue) noexcept
  : SelfDispatchable(std::move(device)),
    physical_device_(physical_device),
    graphics_queue_(graphics_queue),
    present_queue_(present_queue) {}

inline PhysicalDevice Device::GetPhysicalDevice() const noexcept {
  return physical_device_;
}

inline const Queue& Device::GetGraphicsQueue() const noexcept {
  return graphics_queue_;
}

inline const Queue& Device::GetPresentQueue() const noexcept {
  return present_queue_;
}

} // namespace vk

#endif // BACKEND_VK_RENDERER_DEVICE_H_