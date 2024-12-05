#ifndef BACKEND_RENDER_VK_OBJECT_H_
#define BACKEND_RENDER_VK_OBJECT_H_

#include "backend/render/types.h"
#include "backend/render/vk/device.h"
#include "backend/render/vk/config.h"
#include "obj/parser.h"

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

namespace vk {

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

struct UniformBufferObject : render::UniformBufferObject {};

struct DecriptorSetObject {
  Device::Dispatchable<VkDescriptorSetLayout> descriptor_set_layout;
  std::vector<VkDescriptorSet> descriptor_sets;
};

struct Textures : DecriptorSetObject {
  std::vector<Device::Dispatchable<VkImage>> images;
};

struct Uniforms : DecriptorSetObject {
  std::vector<Device::Dispatchable<VkBuffer>> buffers;
};

struct Object {
  Device::Dispatchable<VkBuffer> indices;
  Device::Dispatchable<VkBuffer> vertices;

  std::vector<obj::UseMtl> usemtl;

  Textures textures;
  Uniforms uniforms;

  Device::Dispatchable<VkDescriptorPool> descriptor_pool;
};

class ObjectLoader {
public:
  ObjectLoader() noexcept;
  ObjectLoader(const ObjectLoader& other) = delete;
  ObjectLoader(ObjectLoader&& other) noexcept;

  ObjectLoader(const Device* device, VkCommandPool cmd_pool);
  ~ObjectLoader() = default;

  ObjectLoader& operator=(const ObjectLoader& other) = delete;
  ObjectLoader& operator=(ObjectLoader&& other) noexcept;

  [[nodiscard]] Object Load(const std::string& path) const;
private:
  [[nodiscard]] std::vector<VkDescriptorSet> CreateImagesDescriptorSets(const std::vector<Device::Dispatchable<VkImage>>& images, VkDescriptorSetLayout descriptor_set_layout, VkDescriptorPool descriptor_pool) const;
  [[nodiscard]] std::vector<VkDescriptorSet> CreateBuffersDescriptorSets(const std::vector<Device::Dispatchable<VkBuffer>>& buffers, VkDescriptorSetLayout descriptor_set_layout, VkDescriptorPool descriptor_pool) const;

  [[nodiscard]] Device::Dispatchable<VkBuffer> CreateStagingBuffer(const Device::Dispatchable<VkBuffer>& transfer_buffer, VkBufferUsageFlags usage) const;
  [[nodiscard]] Device::Dispatchable<VkImage> CreateDummyImage(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] Device::Dispatchable<VkImage> CreateStaginImageFromPixels(const unsigned char* pixels, VkExtent2D extent, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const ImageSettings& image_settings) const;
  [[nodiscard]] std::optional<Device::Dispatchable<VkImage>> CreateStagingImage(const std::string& path, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] std::vector<Device::Dispatchable<VkImage>> CreateStagingImages(const obj::Data& data) const;

  const Device* device_;
  VkCommandPool cmd_pool_;
  VkQueue graphics_queue_;
};

} // namespace vk

#endif // BACKEND_RENDER_VK_OBJECT_H_