#include "backend/render/vk/object_loader.h"

#include <cstring>
#include <memory>
#include <utility>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "backend/render/data_util.h"
#include "backend/render/vk/commander.h"
#include "backend/render/vk/error.h"

namespace render::vk {

namespace {

std::pair<Device::Dispatchable<VkBuffer>, Device::Dispatchable<VkBuffer>> CreateTransferBuffers(const obj::Data& data, const Device* device) {
  Device::Dispatchable<VkBuffer> transfer_vertices = device->CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(Vertex) * data.indices.size());
  transfer_vertices.Allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  transfer_vertices.Bind();

  Device::Dispatchable<VkBuffer> transfer_indices = device->CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(Index) * data.indices.size());
  transfer_indices.Allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  transfer_indices.Bind();

  auto mapped_vertices = static_cast<Vertex*>(transfer_vertices.Map());
  auto mapped_indices = static_cast<Index*>(transfer_indices.Map());

  data_util::RemoveDuplicates(data, mapped_vertices, mapped_indices);

  transfer_vertices.Unmap();
  transfer_indices.Unmap();

  return {std::move(transfer_vertices), std::move(transfer_indices)};
}

inline uint32_t CalculateMipMaps(const VkExtent2D extent) {
    return static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1;
}

} // namespace

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

std::vector<Device::Dispatchable<VkImage>> ObjectLoader::CreateStagingImages(const obj::Data& data) const {
  stbi_set_flip_vertically_on_load(true);

  VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  std::vector<Device::Dispatchable<VkImage>> textures;
  textures.reserve(data.mtl.size());

  for(const obj::NewMtl& mtl : data.mtl) {
    Device::Dispatchable<VkImage> texture;
    const std::string& path = mtl.map_kd;
    if (std::optional<Device::Dispatchable<VkImage>> opt_texture = CreateStagingImage(path, usage, properties);
        !opt_texture.has_value()) {
      texture = CreateDummyImage(usage, properties);
    } else {
      texture = std::move(opt_texture.value());
    }
    textures.emplace_back(std::move(texture));
  }
  return textures;
}

Device::Dispatchable<VkImage> ObjectLoader::CreateStagingImageFromPixels(const unsigned char* pixels, VkExtent2D extent, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const {
  VkDeviceSize image_size = extent.width * extent.height * image_settings_.stbi_format;

  Device::Dispatchable<VkBuffer> transfer_buffer = device_->CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, image_size);
  transfer_buffer.Allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  transfer_buffer.Bind();

  void* mapped_buffer = transfer_buffer.Map();
  std::memcpy(mapped_buffer, pixels, static_cast<size_t>(image_size));
  transfer_buffer.Unmap();

  Device::Dispatchable<VkImage> image = device_->CreateImage(usage, extent, image_settings_.vk_format, VK_IMAGE_TILING_OPTIMAL,
                                                             CalculateMipMaps(extent));
  image.Allocate(properties);
  image.Bind();
  image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
  if (!image.FormatFeatureSupported(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    throw Error("image format does not support linear blitting");
  }
  image.CreateSampler(VK_SAMPLER_MIPMAP_MODE_LINEAR);

  ImageCommander commander(image, cmd_pool_, graphics_queue_);
  CommanderGuard commander_guard(commander);

  commander.TransitImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  commander.CopyBuffer(transfer_buffer);
  commander.GenerateMipmaps();

  return image;
}

Device::Dispatchable<VkImage> ObjectLoader::CreateDummyImage(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const {
  std::vector<unsigned char> dummy_colors(image_settings_.dummy_image_extent.width * image_settings_.dummy_image_extent.height, 0xff);

  return CreateStagingImageFromPixels(dummy_colors.data(), image_settings_.dummy_image_extent, usage, properties);
}

std::optional<Device::Dispatchable<VkImage>> ObjectLoader::CreateStagingImage(const std::string& path, const VkBufferUsageFlags usage,
    const VkMemoryPropertyFlags properties) const {
  int image_width, image_height, image_channels;
  std::unique_ptr<stbi_uc, void(*)(void*)> pixels(stbi_load(path.c_str(), &image_width, &image_height, &image_channels, image_settings_.stbi_format), stbi_image_free);
  if (pixels == nullptr) {
    return std::nullopt;
  }

  const VkExtent2D image_extent = { static_cast<uint32_t>(image_width), static_cast<uint32_t>(image_height) };

  return CreateStagingImageFromPixels(pixels.get(), image_extent, usage, properties);
}

inline Device::Dispatchable<VkBuffer> ObjectLoader::CreateStagingBuffer(const Device::Dispatchable<VkBuffer>& transfer_buffer, VkBufferUsageFlags usage) const {
  Device::Dispatchable<VkBuffer> buffer = device_->CreateBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, transfer_buffer.Size());
  buffer.Allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  buffer.Bind();

  BufferCommander commander(buffer, cmd_pool_, graphics_queue_);
  CommanderGuard commander_guard(commander);

  commander.CopyBuffer(transfer_buffer);

  return buffer;
}

std::vector<VkDescriptorSet> ObjectLoader::CreateImagesDescriptorSets(const std::vector<Device::Dispatchable<VkImage>>& images, VkDescriptorSetLayout descriptor_set_layout, VkDescriptorPool descriptor_pool) const {
  const size_t count = images.size();

  std::vector layouts(count, descriptor_set_layout);
  VkDescriptorSetAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = descriptor_pool;
  alloc_info.descriptorSetCount = static_cast<uint32_t>(count);
  alloc_info.pSetLayouts = layouts.data();

  std::vector<VkDescriptorSet> descriptor_sets(count);
  if (const VkResult result = vkAllocateDescriptorSets(device_->Logical(), &alloc_info, descriptor_sets.data()); result != VK_SUCCESS) {
    throw Error("failed to allocate descriptor sets").WithCode(result);
  }
  for (size_t i = 0; i < count; ++i) {
    VkDescriptorImageInfo image_info = {};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = images[i].View().Handle();
    image_info.sampler = images[i].Sampler().Handle();

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptor_sets[i];
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &image_info;

    vkUpdateDescriptorSets(device_->Logical(), 1, &descriptorWrite, 0, nullptr);
  }
  return descriptor_sets;
}

std::vector<VkDescriptorSet> ObjectLoader::CreateBuffersDescriptorSets(const std::vector<Device::Dispatchable<VkBuffer>>& buffers, VkDescriptorSetLayout descriptor_set_layout, VkDescriptorPool descriptor_pool) const {
  const size_t count = buffers.size();

  std::vector layouts(count, descriptor_set_layout);
  VkDescriptorSetAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = descriptor_pool;
  alloc_info.descriptorSetCount = static_cast<uint32_t>(count);
  alloc_info.pSetLayouts = layouts.data();

  std::vector<VkDescriptorSet> descriptor_sets(count);
  if (const VkResult result = vkAllocateDescriptorSets(device_->Logical(), &alloc_info, descriptor_sets.data()); result != VK_SUCCESS) {
    throw Error("failed to allocate descriptor sets").WithCode(result);
  }
  for (size_t i = 0; i < count; ++i) {
    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = buffers[i].Handle();
    buffer_info.offset = 0;
    buffer_info.range = sizeof(Uniforms);

    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptor_sets[i];
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &buffer_info;

    vkUpdateDescriptorSets(device_->Logical(), 1, &descriptor_write, 0, nullptr);
  }
  return descriptor_sets;
}

ObjectLoader::ObjectLoader(const Device* device, ImageSettings image_settings, VkCommandPool cmd_pool)
  : device_(device),
    image_settings_(image_settings),
    cmd_pool_(cmd_pool),
    graphics_queue_(device_->GraphicsQueue()) {}


Object ObjectLoader::Load(const std::string& path, const size_t frame_count) const {
  obj::Data data = obj::ParseFromFile(path);

  auto[transfer_vertices, transfer_indices] = CreateTransferBuffers(data, device_);

  Object object = {};

  object.vertices = CreateStagingBuffer(transfer_vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  object.indices = CreateStagingBuffer(transfer_indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  object.usemtl = std::move(data.usemtl);

  std::vector<Device::Dispatchable<VkImage>> texture_images = CreateStagingImages(data);
  std::vector<Device::Dispatchable<VkBuffer>> ubo_buffers;

  ubo_buffers.resize(frame_count);
  for(Device::Dispatchable<VkBuffer>& ubo_buffer : ubo_buffers) {
    ubo_buffer = device_->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Uniforms));
    ubo_buffer.Allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    ubo_buffer.Bind();
  }
  object.descriptor_pool = device_->CreateDescriptorPool(ubo_buffers.size(), texture_images.size());

  object.ubo.buffers = std::move(ubo_buffers);
  object.ubo.descriptor_set_layout = device_->CreateUboDescriptorSetLayout();
  object.ubo.descriptor_sets = CreateBuffersDescriptorSets(object.ubo.buffers, object.ubo.descriptor_set_layout.Handle(), object.descriptor_pool.Handle());

  object.tbo.images = std::move(texture_images);
  object.tbo.descriptor_set_layout = device_->CreateSamplerDescriptorSetLayout();
  object.tbo.descriptor_sets = CreateImagesDescriptorSets(object.tbo.images, object.tbo.descriptor_set_layout.Handle(), object.descriptor_pool.Handle());

  return object;
}

} // namespace vk