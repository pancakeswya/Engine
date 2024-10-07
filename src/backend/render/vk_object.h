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
  std::vector<Buffer> ubo_buffers;
};

class ObjectLoader {
public:
  DECL_UNIQUE_OBJECT(ObjectLoader);

  ObjectLoader(VkDevice logical_device, VkPhysicalDevice physical_device, VkCommandPool cmd_pool, VkQueue graphics_queue);
  ~ObjectLoader() = default;

  [[nodiscard]] Object Load(const std::string& path) const;
private:
  friend class ObjectFactory;

  [[nodiscard]] Buffer CreateStagingBuffer(const Buffer& transfer_buffer, VkBufferUsageFlags usage) const;
  [[nodiscard]] Image CreateDummyImage(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] Image CreateStaginImageFromPixels(const unsigned char* pixels, VkExtent2D extent, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const ImageSettings& image_settings) const;
  [[nodiscard]] std::optional<Image> CreateStagingImage(const std::string& path, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] std::vector<Image> CreateStagingImages(const obj::Data& data) const;

  VkDevice logical_device_;
  VkPhysicalDevice physical_device_;
  VkCommandPool cmd_pool_;
  VkQueue graphics_queue_;
};

} // namespace vk

#endif // BACKEND_RENDER_VK_OBJECT_H_