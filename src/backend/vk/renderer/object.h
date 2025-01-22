#ifndef BACKEND_VK_RENDERER_OBJECT_H_
#define BACKEND_VK_RENDERER_OBJECT_H_

#include <vulkan/vulkan.h>

#include <vector>

#include "engine/render/types.h"

#include "backend/vk/renderer/dispatchable.h"
#include "backend/vk/renderer/buffer.h"
#include "backend/vk/renderer/image.h"
#include "obj/types.h"

namespace vk {

struct Vertex : engine::Vertex {
  static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
  static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
};

using Index = engine::Index;

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

struct Uniforms : engine::Uniforms {};

struct UniformDescriptorSet {
  Buffer buffer;
  VkDescriptorSet handle;

  void Update() const {
    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = buffer.GetHandle();
    buffer_info.offset = 0;
    buffer_info.range = sizeof(Uniforms);

    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = handle;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &buffer_info;

    vkUpdateDescriptorSets(buffer.GetDevice(), 1, &descriptor_write, 0, nullptr);
  }
};

struct SamplerDescriptorSet {
  DeviceDispatchable<VkSampler> sampler;
  Image image;
  VkDescriptorSet handle;

  void Update() const {
    VkDescriptorImageInfo image_info = {};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = image.GetView();
    image_info.sampler = sampler.GetHandle();

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = handle;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &image_info;

    vkUpdateDescriptorSets(image.GetDevice(), 1, &descriptorWrite, 0, nullptr);
  }
};

struct UniformDescriptor {
  std::vector<UniformDescriptorSet> sets;
  DeviceDispatchable<VkDescriptorSetLayout> layout;
};

struct SamplerDescriptor {
  std::vector<SamplerDescriptorSet> sets;
  DeviceDispatchable<VkDescriptorSetLayout> layout;
};

struct Object {
  Buffer indices;
  Buffer vertices;

  std::vector<obj::UseMtl> usemtl;

  UniformDescriptor uniform_descriptor;
  SamplerDescriptor sampler_descriptor;

  DeviceDispatchable<VkDescriptorPool> descriptor_pool;
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_OBJECT_H_