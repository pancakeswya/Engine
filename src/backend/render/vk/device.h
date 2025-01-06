#ifndef BACKEND_RENDER_VK_DEVICE_H_
#define BACKEND_RENDER_VK_DEVICE_H_

#include "backend/render/vk/dispatchable.h"
#include "backend/render/vk/shaders.h"
#include "backend/window/window.h"

#include <vulkan/vulkan.h>

#include <string_view>
#include <vector>
#include <utility>

namespace render::vk {

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
  ~Device() = default;

  [[nodiscard]] VkDevice Logical() const noexcept;
  [[nodiscard]] VkPhysicalDevice Physical() const noexcept;

  [[nodiscard]] VkQueue GraphicsQueue() const noexcept;
  [[nodiscard]] VkQueue PresentQueue() const noexcept;

  [[nodiscard]] Dispatchable<VkShaderModule> CreateShaderModule(const Shader& shader) const;
  [[nodiscard]] Dispatchable<VkRenderPass> CreateRenderPass(VkFormat image_format, VkFormat depth_format) const;
  [[nodiscard]] Dispatchable<VkPipelineLayout> CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts) const;
  [[nodiscard]] Dispatchable<VkPipeline> CreatePipeline(VkPipelineLayout pipeline_layout, VkRenderPass render_pass, const std::vector<VkDynamicState>& dynamic_states, const std::vector<VkVertexInputAttributeDescription>& attribute_descriptions, const std::vector<VkVertexInputBindingDescription>& binding_descriptions, const std::vector<Dispatchable<VkShaderModule>>& shaders) const;
  [[nodiscard]] Dispatchable<VkCommandPool> CreateCommandPool() const;
  [[nodiscard]] Dispatchable<VkSemaphore> CreateSemaphore() const;
  [[nodiscard]] Dispatchable<VkFence> CreateFence() const;
  [[nodiscard]] Dispatchable<VkDescriptorSetLayout> CreateUboDescriptorSetLayout() const;
  [[nodiscard]] Dispatchable<VkDescriptorSetLayout> CreateSamplerDescriptorSetLayout() const;
  [[nodiscard]] Dispatchable<VkDescriptorPool> CreateDescriptorPool(size_t ubo_count, size_t texture_count) const;
  [[nodiscard]] Dispatchable<VkBuffer> CreateBuffer(VkBufferUsageFlags usage, uint32_t data_size) const;
  [[nodiscard]] Dispatchable<VkImage> CreateImage(VkImageUsageFlags usage,
                                                  VkExtent2D extent,
                                                  VkFormat format,
                                                  VkImageTiling tiling) const;
  [[nodiscard]] Dispatchable<VkSwapchainKHR> CreateSwapchain(window::Size, VkSurfaceKHR surface) const;
  [[nodiscard]] std::vector<VkCommandBuffer> CreateCommandBuffers(VkCommandPool cmd_pool, uint32_t count) const;
private:
  friend class DeviceSelector;

  Device(VkPhysicalDevice physical_device, const QueueFamilyIndices& indices, const std::vector<const char*>& extensions, const VkAllocationCallbacks* allocator = nullptr);

  VkDevice logical_device_;
  VkPhysicalDevice physical_device_;
  const VkAllocationCallbacks* allocator_;
  QueueFamilyIndices indices_;
};

template<>
class Device::Dispatchable<VkBuffer> : public vk::Dispatchable<VkBuffer, Device> {
public:
  using Base = vk::Dispatchable<VkBuffer, Device>;

  Dispatchable() noexcept;
  Dispatchable(const Dispatchable& other) = delete;
  Dispatchable(Dispatchable&& other) noexcept;
  Dispatchable(VkBuffer buffer,
               VkDevice logical_device,
               VkPhysicalDevice physical_device,
               const VkAllocationCallbacks* allocator,
               uint32_t size) noexcept;
  ~Dispatchable() override = default;

  Dispatchable& operator=(const Dispatchable& other) = delete;
  Dispatchable& operator=(Dispatchable&& other) noexcept;

  void Bind() const;
  void Allocate(VkMemoryPropertyFlags properties);
  [[nodiscard]] void* Map() const;
  void Unmap() const noexcept;

  [[nodiscard]] uint32_t Size() const noexcept;
private:
  Dispatchable<VkDeviceMemory> memory_;
  VkPhysicalDevice physical_device_;

  uint32_t size_;
};

template<>
class Device::Dispatchable<VkImage> : public vk::Dispatchable<VkImage, Device> {
public:
  using Base = vk::Dispatchable<VkImage, Device>;

  Dispatchable() noexcept;
  Dispatchable(const Dispatchable& other) = delete;
  Dispatchable(Dispatchable&& other) noexcept;
  Dispatchable(VkImage image,
               VkDevice logical_device,
               VkPhysicalDevice physical_device,
               const VkAllocationCallbacks* allocator,
               VkExtent2D extent,
               VkFormat format,
               uint32_t mip_levels) noexcept;
  ~Dispatchable() override = default;

  Dispatchable& operator=(const Dispatchable&) = delete;
  Dispatchable& operator=(Dispatchable&& other) noexcept;

  void Bind() const;
  void Allocate(VkMemoryPropertyFlags properties);
  void CreateView(VkImageAspectFlags aspect_flags);
  void CreateSampler(VkSamplerMipmapMode mipmap_mode);
  [[nodiscard]] bool FormatFeatureSupported(VkFormatFeatureFlagBits feature) const;
  [[nodiscard]] const Dispatchable<VkImageView>& View() const noexcept;
  [[nodiscard]] const Dispatchable<VkSampler>& Sampler() const noexcept;
  [[nodiscard]] VkFormat Format() const noexcept;
  [[nodiscard]] VkExtent2D Extent() const noexcept;
  [[nodiscard]] uint32_t MipLevels() const noexcept;
private:
  uint32_t mip_levels_;

  VkPhysicalDevice physical_device_;
  VkExtent2D extent_;
  VkFormat format_;

  Dispatchable<VkImageView> view_;
  Dispatchable<VkSampler> sampler_;
  Dispatchable<VkDeviceMemory> memory_;
};

template<>
class Device::Dispatchable<VkShaderModule> : public vk::Dispatchable<VkShaderModule, Device> {
public:
  using Base = vk::Dispatchable<VkShaderModule, Device>;

  Dispatchable() noexcept;
  Dispatchable(const Dispatchable& other) = delete;
  Dispatchable(Dispatchable&& other) noexcept;
  Dispatchable(VkShaderModule module,
               VkDevice logical_device,
               const VkAllocationCallbacks* allocator,
               VkShaderStageFlagBits type,
               std::string_view entry_point) noexcept;
  ~Dispatchable() override = default;

  Dispatchable& operator=(const Dispatchable& other) = delete;
  Dispatchable& operator=(Dispatchable&& other) noexcept;

  [[nodiscard]] VkShaderStageFlagBits Stage() const noexcept;
  [[nodiscard]] std::string_view EntryPoint() const noexcept;
private:
  VkShaderStageFlagBits stage_;
  std::string_view entry_point_;
};

template<>
class Device::Dispatchable<VkSwapchainKHR> : public vk::Dispatchable<VkSwapchainKHR, Device> {
public:
  using Base = vk::Dispatchable<VkSwapchainKHR, Device>;

  Dispatchable() noexcept;
  Dispatchable(const Dispatchable& other) = delete;
  Dispatchable(Dispatchable&& other) noexcept;
  Dispatchable(VkSwapchainKHR swapchain,
               VkDevice logical_device,
               VkPhysicalDevice physical_device,
               const VkAllocationCallbacks* allocator,
               VkExtent2D extent,
               VkFormat format) noexcept;

  ~Dispatchable() override = default;

  Dispatchable& operator=(const Dispatchable&) = delete;
  Dispatchable& operator=(Dispatchable&& other) noexcept;

  [[nodiscard]] VkExtent2D ImageExtent() const noexcept;
  [[nodiscard]] VkFormat ImageFormat() const noexcept;
  [[nodiscard]] VkFormat DepthImageFormat() const noexcept;

  [[nodiscard]] std::vector<Dispatchable<VkFramebuffer>> CreateFramebuffers(VkRenderPass render_pass) const;
private:
  VkExtent2D extent_;
  VkFormat format_;

  VkPhysicalDevice physical_device_;
  Dispatchable<VkImage> depth_image_;
  std::vector<Dispatchable<VkImageView>> image_views_;

  [[nodiscard]] Dispatchable<VkImage> CreateDepthImage() const;
  [[nodiscard]] std::vector<VkImage> GetImages() const;
  [[nodiscard]] std::vector<Dispatchable<VkImageView>> CreateImageViews() const;
  [[nodiscard]] Dispatchable<VkFramebuffer> CreateFramebuffer(const std::vector<VkImageView>& views, VkRenderPass render_pass) const;
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

inline const Device::Dispatchable<VkImageView>& Device::Dispatchable<VkImage>::View() const noexcept {
  return view_;
}

inline const Device::Dispatchable<VkSampler>& Device::Dispatchable<VkImage>::Sampler() const noexcept {
  return sampler_;
}

inline VkFormat Device::Dispatchable<VkImage>::Format() const noexcept {
  return format_;
}

inline VkExtent2D Device::Dispatchable<VkImage>::Extent() const noexcept {
  return extent_;
}

inline uint32_t Device::Dispatchable<VkImage>::MipLevels() const noexcept {
  return mip_levels_;
}

inline VkExtent2D Device::Dispatchable<VkSwapchainKHR>::ImageExtent() const noexcept {
  return extent_;
}

inline VkFormat Device::Dispatchable<VkSwapchainKHR>::ImageFormat() const noexcept {
  return format_;
}

inline VkFormat Device::Dispatchable<VkSwapchainKHR>::DepthImageFormat() const noexcept {
  return depth_image_.Format();
}

inline VkShaderStageFlagBits Device::Dispatchable<VkShaderModule>::Stage() const noexcept {
  return stage_;
}

inline std::string_view Device::Dispatchable<VkShaderModule>::EntryPoint() const noexcept {
  return entry_point_;
}

} // namespace vk

#endif // BACKEND_RENDER_VK_DEVICE_H_