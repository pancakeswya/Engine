#include "backend/render/vk_object.h"
#include "backend/render/vk_config.h"
#include "backend/render/vk_commander.h"
#include "backend/render/vk_factory.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <array>
#include <algorithm>
#include <cstring>
#include <unordered_map>

namespace vk {

namespace {

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
  Buffer transfer_vertices(logical_device, physical_device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(Vertex) * data.indices.size());
  transfer_vertices.Allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  transfer_vertices.Bind();

  Buffer transfer_indices(logical_device, physical_device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(Index::type) * data.indices.size());
  transfer_indices.Allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
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

  Buffer transfer_buffer(logical_device_, physical_device_, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, image_size);
  transfer_buffer.Allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  transfer_buffer.Bind();

  void* mapped_buffer = transfer_buffer.Map();
  std::memcpy(mapped_buffer, pixels, static_cast<size_t>(image_size));
  transfer_buffer.Unmap();

  Image image(logical_device_, physical_device_, extent, image_settings.channels, image_settings.format, VK_IMAGE_TILING_OPTIMAL, usage);
  image.Allocate(properties);
  image.Bind();
  image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
  if (!image.FormatFeatureSupported(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    throw Error("image format does not support linear blitting");
  }
  image.CreateSampler(VK_SAMPLER_MIPMAP_MODE_LINEAR);

  ImageCommander commander(image, cmd_pool_, graphics_queue_);
  CommandGuard command_guard(commander);

  commander.TransitImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  commander.CopyBuffer(transfer_buffer);
  commander.GenerateMipmaps();

  return image;
}

Image ObjectLoader::CreateDummyImage(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const {
  ImageSettings image_settings = config::GetImageSettings();

  std::array<unsigned char, config::kDummyImageExtent.width * config::kDummyImageExtent.height> dummy_colors;
  std::fill(dummy_colors.begin(), dummy_colors.end(), 0xff);

  return CreateStaginImageFromPixels(dummy_colors.data(), config::kDummyImageExtent, usage, properties, image_settings);
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
  Buffer buffer(logical_device_, physical_device_, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, transfer_buffer.Size());
  buffer.Allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  buffer.Bind();

  BufferCommander commander(buffer, cmd_pool_, graphics_queue_);
  CommandGuard command_guard(commander);

  commander.CopyBuffer(transfer_buffer);

  return buffer;
}

ObjectLoader::ObjectLoader(VkDevice logical_device, VkPhysicalDevice physical_device, VkCommandPool cmd_pool, VkQueue graphics_queue)
  : logical_device_(logical_device),
    physical_device_(physical_device),
    cmd_pool_(cmd_pool),
    graphics_queue_(graphics_queue) {}

Object ObjectLoader::Load(const std::string& path) const {
  obj::Data data = obj::ParseFromFile(path);

  auto[transfer_vertices, transfer_indices] = CreateTransferBuffers(data, logical_device_, physical_device_);

  Object object = {};

  object.vertices = CreateStagingBuffer(transfer_vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  object.indices = CreateStagingBuffer(transfer_indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  object.textures = CreateStagingImages(data);
  object.usemtl = std::move(data.usemtl);

  object.ubo_buffers.resize(config::kFrameCount);
  for(Buffer& ubo_buffer : object.ubo_buffers) {
    ubo_buffer = Buffer(logical_device_, physical_device_, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(UniformBufferObject));
    ubo_buffer.Allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    ubo_buffer.Bind();
  }

  return object;
}

} // namespace vk