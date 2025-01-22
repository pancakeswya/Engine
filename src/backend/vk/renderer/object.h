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

  void Update() const noexcept;
};

struct SamplerDescriptorSet {
  DeviceDispatchable<VkSampler> sampler;
  Image image;
  VkDescriptorSet handle;

  void Update() const noexcept;
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