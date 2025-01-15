#ifndef BACKEND_VK_RENDERER_DEVICE_H_
#define BACKEND_VK_RENDERER_DEVICE_H_

#include "backend/vk/renderer/dispatchable.h"

#include <vulkan/vulkan.h>

#include <vector>

namespace vk {

class Buffer;
class Image;
class ShaderModule;
class Swapchain;

struct ShaderInfo;

struct QueueFamilyIndices {
  uint32_t graphic, present;
};

struct SurfaceSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

class Device {
public:
  using HandleType = VkDevice;

  template<typename Tp>
  class Dispatchable : public vk::Dispatchable<Tp, Device> {
  public:
    using Base = vk::Dispatchable<Tp, Device>;
    using Base::Base;
  };

  static SurfaceSupportDetails GetSurfaceSupport(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

  Device() = default;
  Device(const Device&) = delete;
  Device(Device&& other) noexcept;
  ~Device();

  Device& operator=(const Device&) = delete;
  Device& operator=(Device&&) noexcept;

  [[nodiscard]] VkDevice Logical() const noexcept;
  [[nodiscard]] VkPhysicalDevice Physical() const noexcept;

  [[nodiscard]] VkQueue GraphicsQueue() const noexcept;
  [[nodiscard]] VkQueue PresentQueue() const noexcept;

  [[nodiscard]] ShaderModule CreateShaderModule(const ShaderInfo& shader_info) const;
  [[nodiscard]] Dispatchable<VkRenderPass> CreateRenderPass(VkFormat image_format, VkFormat depth_format) const;
  [[nodiscard]] Dispatchable<VkPipelineLayout> CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts) const;
  [[nodiscard]] Dispatchable<VkPipeline> CreatePipeline(VkPipelineLayout pipeline_layout, VkRenderPass render_pass, const std::vector<VkVertexInputAttributeDescription>& attribute_descriptions, const std::vector<VkVertexInputBindingDescription>& binding_descriptions, const std::vector<ShaderModule>& shaders) const;
  [[nodiscard]] Dispatchable<VkCommandPool> CreateCommandPool() const;
  [[nodiscard]] Dispatchable<VkSemaphore> CreateSemaphore() const;
  [[nodiscard]] Dispatchable<VkFence> CreateFence() const;
  [[nodiscard]] Dispatchable<VkDescriptorSetLayout> CreateUboDescriptorSetLayout() const;
  [[nodiscard]] Dispatchable<VkDescriptorSetLayout> CreateSamplerDescriptorSetLayout() const;
  [[nodiscard]] Dispatchable<VkDescriptorPool> CreateDescriptorPool(size_t ubo_count, size_t texture_count) const;
  [[nodiscard]] Buffer CreateBuffer(VkBufferUsageFlags usage, uint32_t data_size) const;
  [[nodiscard]] Image CreateImage(VkImageUsageFlags usage,
                                  VkExtent2D extent,
                                  VkFormat format,
                                  VkImageTiling tiling,
                                  uint32_t mip_levels) const;
  [[nodiscard]] Swapchain CreateSwapchain(VkExtent2D size, VkSurfaceKHR surface) const;
  [[nodiscard]] std::vector<VkCommandBuffer> CreateCommandBuffers(VkCommandPool cmd_pool, uint32_t count) const;
private:
  friend class DeviceSelector;

  Device(VkPhysicalDevice physical_device, const QueueFamilyIndices& indices, const std::vector<const char*>& extensions, const VkAllocationCallbacks* allocator = nullptr);

  VkDevice logical_device_;
  VkPhysicalDevice physical_device_;
  const VkAllocationCallbacks* allocator_;
  QueueFamilyIndices indices_;
};

inline VkDevice Device::Logical() const noexcept {
  return logical_device_;
}

inline VkPhysicalDevice Device::Physical() const noexcept {
  return physical_device_;
}

inline VkQueue Device::GraphicsQueue() const noexcept {
  VkQueue graphics_queue = VK_NULL_HANDLE;
  vkGetDeviceQueue(logical_device_, indices_.graphic, 0, &graphics_queue);
  return graphics_queue;
}

inline VkQueue Device::PresentQueue() const noexcept {
  VkQueue present_queue = VK_NULL_HANDLE;
  vkGetDeviceQueue(logical_device_, indices_.present, 0, &present_queue);
  return present_queue;
}

} // namespace vk

#endif // BACKEND_VK_RENDERER_DEVICE_H_