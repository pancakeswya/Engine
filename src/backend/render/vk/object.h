#ifndef BACKEND_RENDER_VK_OBJECT_H_
#define BACKEND_RENDER_VK_OBJECT_H_

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

#include "backend/render/types.h"
#include "backend/render/vk/config.h"
#include "backend/render/vk/device.h"
#include "obj/parser.h"

namespace render::vk {

struct Vertex : render::Vertex {
  static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
  static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
};

using Index = render::Index;

template<typename Tp>
struct IndexType;

template<>
struct IndexType<uint16_t> {
  static constexpr VkIndexType value = VK_INDEX_TYPE_UINT16;
};

template<>
struct IndexType<uint32_t> {
  static constexpr VkIndexType value = VK_INDEX_TYPE_UINT32;
};

struct Uniforms : render::Uniforms {};

struct DescriptorSetObject {
  Device::Dispatchable<VkDescriptorSetLayout> descriptor_set_layout;
  std::vector<VkDescriptorSet> descriptor_sets;
};

struct TextureBufferObject : DescriptorSetObject {
  std::vector<Device::Dispatchable<VkImage>> images;
};

struct UniformBufferObject : DescriptorSetObject {
  std::vector<Device::Dispatchable<VkBuffer>> buffers;
};

struct Object {
  Device::Dispatchable<VkBuffer> indices;
  Device::Dispatchable<VkBuffer> vertices;

  std::vector<obj::UseMtl> usemtl;

  TextureBufferObject tbo;
  UniformBufferObject ubo;

  Device::Dispatchable<VkDescriptorPool> descriptor_pool;
};

class ObjectLoader {
public:
  ObjectLoader() noexcept = default;
  ObjectLoader(const Device* device, ImageSettings image_settings, VkCommandPool cmd_pool);
  ~ObjectLoader() = default;

  [[nodiscard]] Object Load(const std::string& path, size_t frame_count) const;
private:
  [[nodiscard]] std::vector<VkDescriptorSet> CreateImagesDescriptorSets(const std::vector<Device::Dispatchable<VkImage>>& images, VkDescriptorSetLayout descriptor_set_layout, VkDescriptorPool descriptor_pool) const;
  [[nodiscard]] std::vector<VkDescriptorSet> CreateBuffersDescriptorSets(const std::vector<Device::Dispatchable<VkBuffer>>& buffers, VkDescriptorSetLayout descriptor_set_layout, VkDescriptorPool descriptor_pool) const;

  [[nodiscard]] Device::Dispatchable<VkBuffer> CreateStagingBuffer(const Device::Dispatchable<VkBuffer>& transfer_buffer, VkBufferUsageFlags usage) const;
  [[nodiscard]] Device::Dispatchable<VkImage> CreateDummyImage(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] Device::Dispatchable<VkImage> CreateStaginImageFromPixels(const unsigned char* pixels, VkExtent2D extent, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] std::optional<Device::Dispatchable<VkImage>> CreateStagingImage(const std::string& path, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] std::vector<Device::Dispatchable<VkImage>> CreateStagingImages(const obj::Data& data) const;

  const Device* device_;
  ImageSettings image_settings_;
  VkCommandPool cmd_pool_;
  VkQueue graphics_queue_;
};

} // namespace vk

#endif // BACKEND_RENDER_VK_OBJECT_H_