#ifndef BACKEND_RENDER_VK_OBJECT_H_
#define BACKEND_RENDER_VK_OBJECT_H_

#include "backend/render/vk_types.h"
#include "backend/render/vk_wrappers.h"
#include "obj/parser.h"

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

namespace vk {

struct Object {
  Buffer indices;
  Buffer vertices;
  std::vector<Image> textures;
  std::vector<obj::UseMtl> usemtl;

  HandleWrapper<VkDescriptorSetLayout> descriptor_set_layout_wrapper;
  std::vector<VkDescriptorSet> descriptor_sets;
  std::vector<UniformBufferObject*> ubo_mapped;

  HandleWrapper<VkDescriptorPool> descriptor_pool_wrapper;
  std::vector<Buffer> ubo_buffers;
};

class ObjectLoader {
public:
  void Load(const std::string& path, Object& object) const;
private:
  [[nodiscard]] Buffer CreateStagingBuffer(const Buffer& transfer_buffer, VkBufferUsageFlags usage) const;
  [[nodiscard]] Image CreateDummyImage(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] Image CreateStaginImageFromPixels(const unsigned char* pixels, VkExtent2D extent, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const ImageSettings& image_settings) const;
  [[nodiscard]] std::optional<Image> CreateStagingImage(const std::string& path, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] std::vector<Image> CreateStagingImages(const obj::Data& data) const;

  friend class ObjectFactory;

  VkDevice logical_device_;
  VkPhysicalDevice physical_device_;
  VkSampler texture_sampler_;
  VkCommandPool cmd_pool_;
  VkQueue graphics_queue_;
};

class ObjectFactory {
public:
  DECL_UNIQUE_OBJECT(ObjectFactory);

  ObjectFactory(VkDevice logical_device, VkPhysicalDevice physical_device);

  [[nodiscard]] Object CreateObject(size_t ubo_count) const;
  [[nodiscard]] ObjectLoader CreateObjectLoader(VkSampler texture_sampler, VkCommandPool cmd_pool, VkQueue graphics_queue) const noexcept;
private:
  VkDevice logical_device_;
  VkPhysicalDevice physical_device_;
};

inline ObjectFactory::ObjectFactory(VkDevice logical_device, VkPhysicalDevice physical_device)
    : logical_device_(logical_device), physical_device_(physical_device) {}

} // namespace vk

#endif // BACKEND_RENDER_VK_OBJECT_H_