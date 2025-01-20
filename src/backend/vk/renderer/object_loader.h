#ifndef BACKEND_VK_RENDERER_OBJECT_LOADER_H_
#define BACKEND_VK_RENDERER_OBJECT_LOADER_H_

#include <optional>
#include <vector>

#include "backend/vk/renderer/config.h"
#include "backend/vk/renderer/device.h"
#include "backend/vk/renderer/object.h"

namespace vk {

class DeviceDispatchableFactory;

class ObjectLoader {
public:
  ObjectLoader(const Device& device, ImageSettings image_settings, VkCommandPool cmd_pool) noexcept;
  ~ObjectLoader() = default;

  [[nodiscard]] Object Load(const std::string& path, size_t frame_count) const;
private:
  [[nodiscard]] std::vector<VkDescriptorSet> CreateImagesDescriptorSets(const std::vector<Image>& images, const std::vector<DeviceDispatchable<VkSampler>>& samplers, VkDescriptorSetLayout descriptor_set_layout, VkDescriptorPool descriptor_pool) const;
  [[nodiscard]] std::vector<VkDescriptorSet> CreateBuffersDescriptorSets(const std::vector<Buffer>& buffers, VkDescriptorSetLayout descriptor_set_layout, VkDescriptorPool descriptor_pool) const;

  [[nodiscard]] Buffer CreateStagingBuffer(const DeviceDispatchableFactory& dispatchable_factory, const Buffer& transfer_buffer, VkBufferUsageFlags usage) const;
  [[nodiscard]] Image CreateDummyImage(const DeviceDispatchableFactory& dispatchable_factory, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] Image CreateStagingImageFromPixels(const DeviceDispatchableFactory& dispatchable_factory, const unsigned char* pixels, VkExtent2D extent, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] std::optional<Image> CreateStagingImage(const DeviceDispatchableFactory& dispatchable_factory, const std::string& path, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] std::vector<Image> CreateStagingImages(const DeviceDispatchableFactory& dispatchable_factory, const obj::Data& data) const;

  const Device& device_;
  ImageSettings image_settings_;
  VkCommandPool cmd_pool_;
  VkQueue graphics_queue_;
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_OBJECT_LOADER_H_