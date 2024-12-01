#ifndef BACKEND_RENDER_VK_OBJECT_H_
#define BACKEND_RENDER_VK_OBJECT_H_

#include "backend/render/vk/device.h"
#include "backend/render/vk/config.h"
#include "obj/parser.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

namespace vk {

struct Vertex {
  glm::vec3 pos;
  glm::vec3 normal;
  glm::vec2 tex_coord;

  static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
  static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
};

struct Index {
  using type = uint32_t;

  static constexpr VkIndexType type_enum = VK_INDEX_TYPE_UINT32;
};

struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

struct Object {
  Device::Dispatchable<VkBuffer> indices;
  Device::Dispatchable<VkBuffer> vertices;

  std::vector<Device::Dispatchable<VkImage>> textures;
  std::vector<obj::UseMtl> usemtl;
  std::vector<Device::Dispatchable<VkBuffer>> ubo_buffers;
};

class ObjectLoader {
public:
  ObjectLoader() noexcept;
  ObjectLoader(ObjectLoader& other) = delete;
  ObjectLoader(ObjectLoader&& other) noexcept;

  ObjectLoader(const Device* device, VkCommandPool cmd_pool);
  ~ObjectLoader() = default;

  ObjectLoader& operator=(ObjectLoader& other) = delete;
  ObjectLoader& operator=(ObjectLoader&& other) noexcept;

  [[nodiscard]] Object Load(const std::string& path) const;
private:
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