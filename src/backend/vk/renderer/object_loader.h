#ifndef BACKEND_VK_RENDERER_OBJECT_LOADER_H_
#define BACKEND_VK_RENDERER_OBJECT_LOADER_H_

#include <optional>
#include <vector>

#include "backend/vk/renderer/config.h"
#include "backend/vk/renderer/device.h"
#include "backend/vk/renderer/object.h"

namespace vk {

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
  [[nodiscard]] Device::Dispatchable<VkImage> CreateStagingImageFromPixels(const unsigned char* pixels, VkExtent2D extent, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] std::optional<Device::Dispatchable<VkImage>> CreateStagingImage(const std::string& path, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] std::vector<Device::Dispatchable<VkImage>> CreateStagingImages(const obj::Data& data) const;

  const Device* device_;
  ImageSettings image_settings_;
  VkCommandPool cmd_pool_;
  VkQueue graphics_queue_;
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_OBJECT_LOADER_H_