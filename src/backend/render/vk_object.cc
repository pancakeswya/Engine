#include "backend/render/vk_object.h"
#include "backend/render/vk_config.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <array>
#include <algorithm>
#include <cstring>
#include <unordered_map>

namespace vk {

namespace {

void UpdateBufferDescriptorSets(VkDevice logical_device, VkBuffer ubo_buffer, VkDescriptorSet descriptor_set) {
  VkDescriptorBufferInfo buffer_info = {};
  buffer_info.buffer = ubo_buffer;
  buffer_info.offset = 0;
  buffer_info.range = sizeof(UniformBufferObject);

  VkWriteDescriptorSet descriptor_write = {};

  descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor_write.dstSet = descriptor_set;
  descriptor_write.dstBinding = 0;
  descriptor_write.dstArrayElement = 0;
  descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptor_write.descriptorCount = 1;
  descriptor_write.pBufferInfo = &buffer_info;

  vkUpdateDescriptorSets(logical_device, 1, &descriptor_write, 0, nullptr);
}

void UpdateTextureDescriptorSets(VkDevice logical_device, const std::vector<Image>& images, VkDescriptorSet descriptor_set) {
  std::vector<VkDescriptorImageInfo> image_infos(images.size());
  for(size_t i = 0; i < images.size(); ++i) {
    image_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_infos[i].imageView = images[i].GetView();
    image_infos[i].sampler = images[i].GetSampler();
  }
  VkWriteDescriptorSet descriptor_write = {};

  descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor_write.dstSet = descriptor_set;
  descriptor_write.dstBinding = 1;
  descriptor_write.dstArrayElement = 0;
  descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptor_write.descriptorCount = image_infos.size();
  descriptor_write.pImageInfo = image_infos.data();

  vkUpdateDescriptorSets(logical_device, 1, &descriptor_write, 0, nullptr);
}

void RemoveDuplicatesAndCopy(const obj::Data& data, Vertex* mapped_vertices, Index::type* mapped_indices) {
  std::unordered_map<obj::Indices, unsigned int, obj::Indices::Hash> index_map;

  unsigned int next_combined_idx = 0, combined_idx = 0;
  for (const obj::Indices& index : data.indices) {
    if (index_map.count(index)) {
      combined_idx = index_map.at(index);
    } else {
      combined_idx = next_combined_idx;
      index_map.emplace(index, combined_idx);
      unsigned int i_v = index.fv * 3, i_n = index.fn * 3, i_t = index.ft * 2;
      *mapped_vertices++ = Vertex{
          {data.v[i_v], data.v[i_v + 1], data.v[i_v + 2]},
          {data.vn[i_n], data.vn[i_n + 1], data.vn[i_n + 2]},
          {data.vt[i_t], data.vt[i_t + 1]}
      };
      ++next_combined_idx;
    }
    *mapped_indices++ = combined_idx;
  }
}

std::pair<Buffer, Buffer> CreateTransferBuffers(const obj::Data& data, VkDevice logical_device, VkPhysicalDevice physical_device) {
  Buffer transfer_vertices(logical_device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(Vertex) * data.indices.size());
  transfer_vertices.Allocate(physical_device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  transfer_vertices.Bind();

  Buffer transfer_indices(logical_device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(Index::type) * data.indices.size());
  transfer_indices.Allocate(physical_device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  transfer_indices.Bind();

  auto mapped_vertices = static_cast<Vertex*>(transfer_vertices.Map());
  auto mapped_indices = static_cast<Index::type*>(transfer_indices.Map());

  RemoveDuplicatesAndCopy(data, mapped_vertices, mapped_indices);

  transfer_vertices.Unmap();
  transfer_indices.Unmap();

  return {std::move(transfer_vertices), std::move(transfer_indices)};
}

} // namespace

std::vector<Image> ObjectLoader::CreateStagingImages(const obj::Data& data) const {
  stbi_set_flip_vertically_on_load(true);

  VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  std::vector<Image> textures;
  textures.reserve(data.mtl.size());

  for(const obj::NewMtl& mtl : data.mtl) {
    Image texture;
    const std::string& path = mtl.map_kd;
    std::optional<Image> opt_texture = CreateStagingImage(path, usage, properties);
    if (!opt_texture.has_value()) {
      texture = CreateDummyImage(usage, properties);
    } else {
      texture = std::move(opt_texture.value());
    }
    textures.emplace_back(std::move(texture));
  }
  return textures;
}

Image ObjectLoader::CreateStaginImageFromPixels(const unsigned char* pixels, VkExtent2D extent, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const ImageSettings& image_settings) const {
  VkDeviceSize image_size = extent.width * extent.height * image_settings.channels;

  Buffer transfer_buffer(logical_device_, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, image_size);
  transfer_buffer.Allocate(physical_device_, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  transfer_buffer.Bind();

  void* mapped_buffer = transfer_buffer.Map();
  std::memcpy(mapped_buffer, pixels, static_cast<size_t>(image_size));
  transfer_buffer.Unmap();

  Image image(logical_device_, extent, image_settings.channels, image_settings.format, VK_IMAGE_TILING_OPTIMAL, usage);
  image.Allocate(physical_device_, properties);
  image.Bind();

  image.TransitImageLayout(cmd_pool_, graphics_queue_, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  image.CopyBuffer(transfer_buffer, cmd_pool_, graphics_queue_);
  image.TransitImageLayout(cmd_pool_, graphics_queue_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
  image.GenerateMipmaps(physical_device_, cmd_pool_, graphics_queue_);

  return image;
}

Image ObjectLoader::CreateDummyImage(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const {
  ImageSettings image_settings = config::GetImageSettings();

  constexpr uint32_t image_width = 16;
  constexpr uint32_t image_height = 16;

  std::array<unsigned char, image_width * image_height> dummy_colors;
  std::fill(dummy_colors.begin(), dummy_colors.end(), 0xff);

  constexpr VkExtent2D extent = { image_width, image_height };

  return CreateStaginImageFromPixels(dummy_colors.data(), extent, usage, properties, image_settings);
}

std::optional<Image> ObjectLoader::CreateStagingImage(const std::string& path, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const {
  ImageSettings image_settings = config::GetImageSettings();

  int image_width, image_height, image_channels;
  stbi_uc* pixels = stbi_load(path.c_str(), &image_width, &image_height, &image_channels, image_settings.channels);
  if (pixels == nullptr) {
    return std::nullopt;
  }
  HandleWrapper<stbi_uc*> image_wrapper(pixels, [](stbi_uc* pixels) {
    stbi_image_free(pixels);
  });

  const VkExtent2D image_extent = { static_cast<uint32_t>(image_width), static_cast<uint32_t>(image_height) };

  return CreateStaginImageFromPixels(pixels, image_extent, usage, properties, image_settings);
}

inline Buffer ObjectLoader::CreateStagingBuffer(const Buffer& transfer_buffer, VkBufferUsageFlags usage) const {
  Buffer buffer(logical_device_, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, transfer_buffer.Size());
  buffer.Allocate(physical_device_, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  buffer.Bind();
  buffer.CopyBuffer(transfer_buffer, cmd_pool_, graphics_queue_);

  return buffer;
}

void ObjectLoader::Load(const std::string& path, Object& object) const {
  obj::Data data = obj::ParseFromFile(path);

  auto[transfer_vertices, transfer_indices] = CreateTransferBuffers(data, logical_device_, physical_device_);

  object.vertices = CreateStagingBuffer(transfer_vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  object.indices = CreateStagingBuffer(transfer_indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  object.textures = CreateStagingImages(data);
  object.usemtl = std::move(data.usemtl);

  for(VkDescriptorSet descriptor_set : object.descriptor_sets) {
    UpdateTextureDescriptorSets(logical_device_, object.textures, descriptor_set);
  }
}

Object ObjectFactory::CreateObject(size_t ubo_count) const {
  Object object;

  object.descriptor_set_layout_wrapper = factory::CreateDescriptorSetLayout(logical_device_);
  VkDescriptorSetLayout descriptor_set_layout = object.descriptor_set_layout_wrapper.get();

  object.descriptor_pool_wrapper = factory::CreateDescriptorPool(logical_device_, ubo_count);
  VkDescriptorPool descriptor_pool = object.descriptor_pool_wrapper.get();

  object.descriptor_sets = factory::CreateDescriptorSets(logical_device_, descriptor_set_layout, descriptor_pool, ubo_count);

  object.ubo_buffers.resize(ubo_count);
  object.ubo_mapped.resize(ubo_count);

  for(size_t i = 0; i < object.ubo_buffers.size(); ++i) {
    object.ubo_buffers[i] = Buffer(logical_device_, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(UniformBufferObject));
    object.ubo_buffers[i].Allocate(physical_device_, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    object.ubo_buffers[i].Bind();

    object.ubo_mapped[i] = static_cast<UniformBufferObject*>(object.ubo_buffers[i].Map());

    UpdateBufferDescriptorSets(logical_device_, object.ubo_buffers[i].Get(), object.descriptor_sets[i]);
  }
  return object;
}

ObjectLoader ObjectFactory::CreateObjectLoader(VkCommandPool cmd_pool, VkQueue graphics_queue) const noexcept {
  ObjectLoader loader = {};

  loader.logical_device_ = logical_device_;
  loader.physical_device_ = physical_device_;
  loader.cmd_pool_ = cmd_pool;
  loader.graphics_queue_ = graphics_queue;

  return loader;
}

} // namespace vk