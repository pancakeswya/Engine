#include "backend/vk/renderer/object.h"

namespace vk {

std::vector<VkVertexInputBindingDescription> Vertex::GetBindingDescriptions() {
  std::vector<VkVertexInputBindingDescription>binding_descriptions(1);
  binding_descriptions[0].binding = 0;
  binding_descriptions[0].stride = sizeof(Vertex);
  binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return binding_descriptions;
}

std::vector<VkVertexInputAttributeDescription> Vertex::GetAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> attribute_descriptions(3);
  attribute_descriptions[0].binding = 0;
  attribute_descriptions[0].location = 0;
  attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attribute_descriptions[0].offset = offsetof(Vertex, pos);

  attribute_descriptions[1].binding = 0;
  attribute_descriptions[1].location = 1;
  attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attribute_descriptions[1].offset = offsetof(Vertex, normal);

  attribute_descriptions[2].binding = 0;
  attribute_descriptions[2].location = 2;
  attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
  attribute_descriptions[2].offset = offsetof(Vertex, tex_coord);

  return attribute_descriptions;
}

void UniformDescriptorSet::Update() const noexcept {
  VkDescriptorBufferInfo buffer_info = {};
  buffer_info.buffer = buffer.handle();
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

  vkUpdateDescriptorSets(buffer.creator(), 1, &descriptor_write, 0, nullptr);
}

void SamplerDescriptorSet::Update() const noexcept {
  VkDescriptorImageInfo image_info = {};
  image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  image_info.imageView = image.view();
  image_info.sampler = sampler.handle();

  VkWriteDescriptorSet descriptor_write = {};
  descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor_write.dstSet = handle;
  descriptor_write.dstBinding = 0;
  descriptor_write.dstArrayElement = 0;
  descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptor_write.descriptorCount = 1;
  descriptor_write.pImageInfo = &image_info;

  vkUpdateDescriptorSets(image.creator(), 1, &descriptor_write, 0, nullptr);
}

} // namespace vk
