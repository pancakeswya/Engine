#ifndef BACKEND_VK_RENDERER_OBJECT_LOADER_H_
#define BACKEND_VK_RENDERER_OBJECT_LOADER_H_

#include <optional>
#include <vector>
#include <utility>

#include "backend/vk/renderer/config.h"
#include "backend/vk/renderer/device.h"
#include "backend/vk/renderer/object.h"

namespace vk {

class ObjectLoader {
public:
  static void Init() noexcept;

  ObjectLoader(const Device& device, ImageSettings image_settings, VkCommandPool cmd_pool) noexcept;
  ~ObjectLoader() = default;

  [[nodiscard]] Object Load(const std::string& path, size_t frame_count) const;
private:
  [[nodiscard]] std::pair<Buffer, Buffer> CreateTransferBuffers(const obj::Data& data) const;
  [[nodiscard]] Buffer CreateStagingBuffer(const Buffer& transfer_buffer, VkBufferUsageFlags usage) const;
  [[nodiscard]] Image CreateDummyImage(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] Image CreateStagingImageFromPixels(const unsigned char* pixels, VkExtent2D extent, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] std::optional<Image> CreateStagingImage(const std::string& path, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] std::vector<Image> CreateStagingImages(const obj::Data& data) const;
  [[nodiscard]] UniformDescriptor CreateUniformDescriptor(VkDescriptorPool descriptor_pool, size_t frame_count) const;
  [[nodiscard]] SamplerDescriptor CreateSamplerDescriptor(VkDescriptorPool descriptor_pool, std::vector<Image>&& images) const;

  const Device& device_;
  ImageSettings image_settings_;
  VkCommandPool cmd_pool_;
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_OBJECT_LOADER_H_