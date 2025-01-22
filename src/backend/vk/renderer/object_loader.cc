#include "backend/vk/renderer/object_loader.h"

#include <cstring>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "engine/render/data_util.h"
#include "backend/vk/renderer/commander.h"
#include "backend/vk/renderer/error.h"
#include "obj/parser.h"

namespace vk {

namespace {

inline uint32_t CalculateMipMaps(const VkExtent2D extent) {
    return static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1;
}

} // namespace

void ObjectLoader::Init() noexcept {
  stbi_set_flip_vertically_on_load(true);
}

ObjectLoader::ObjectLoader(const Device& device, ImageSettings image_settings, VkCommandPool cmd_pool) noexcept
  : device_(device),
    image_settings_(image_settings),
    cmd_pool_(cmd_pool) {}

Object ObjectLoader::Load(const std::string& path, const size_t frame_count) const {
  obj::Data data = obj::ParseFromFile(path);

  auto[transfer_vertices, transfer_indices] = CreateTransferBuffers(data);

  Object object = {};
  object.vertices = CreateStagingBuffer(transfer_vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  object.indices = CreateStagingBuffer(transfer_indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  object.usemtl = std::move(data.usemtl);

  std::vector<Image> images = CreateStagingImages(data);

  object.descriptor_pool = device_.CreateDescriptorPool(frame_count, images.size());
  object.uniform_descriptor = CreateUniformDescriptor(object.descriptor_pool.GetHandle(), frame_count);
  object.sampler_descriptor = CreateSamplerDescriptor(object.descriptor_pool.GetHandle(), std::move(images));

  return object;
}

std::pair<Buffer, Buffer> ObjectLoader::CreateTransferBuffers(const obj::Data& data) const {
  Buffer transfer_vertices = device_.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(Vertex) * data.indices.size());
  device_.AllocateBuffer(transfer_vertices, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  device_.BindBuffer(transfer_vertices);

  Buffer transfer_indices = device_.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, sizeof(Index) * data.indices.size());
  device_.AllocateBuffer(transfer_indices, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  device_.BindBuffer(transfer_indices);

  auto mapped_vertices = static_cast<Vertex*>(transfer_vertices.GetMemory().Map());
  auto mapped_indices = static_cast<Index*>(transfer_indices.GetMemory().Map());

  engine::data_util::RemoveDuplicates(data, mapped_vertices, mapped_indices);

  transfer_vertices.GetMemory().Unmap();
  transfer_indices.GetMemory().Unmap();

  return {std::move(transfer_vertices), std::move(transfer_indices)};
}

inline Buffer ObjectLoader::CreateStagingBuffer(const Buffer& transfer_buffer, const VkBufferUsageFlags usage) const {
  Buffer buffer = device_.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, transfer_buffer.Size());
  device_.AllocateBuffer(buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  device_.BindBuffer(buffer);

  BufferCommander commander(buffer, cmd_pool_, device_.GetGraphicsQueue().handle);
  CommanderGuard commander_guard(commander);

  commander.CopyBuffer(transfer_buffer);

  return buffer;
}

Image ObjectLoader::CreateStagingImageFromPixels(const unsigned char* pixels, const VkExtent2D extent, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties) const {
  const VkDeviceSize image_size = extent.width * extent.height * image_settings_.stbi_format;

  Buffer transfer_buffer = device_.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, image_size);
  device_.AllocateBuffer(transfer_buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  device_.BindBuffer(transfer_buffer);

  void* mapped_buffer = transfer_buffer.GetMemory().Map();
  std::memcpy(mapped_buffer, pixels, image_size);
  transfer_buffer.GetMemory().Unmap();

  Image image = device_.CreateImage(usage, extent, image_settings_.vk_format, VK_IMAGE_TILING_OPTIMAL, CalculateMipMaps(extent));
  device_.AllocateImage(image, properties);
  device_.BindImage(image);
  device_.MakeImageView(image, VK_IMAGE_ASPECT_COLOR_BIT);

  ImageCommander commander(image, cmd_pool_, device_.GetGraphicsQueue().handle);
  CommanderGuard commander_guard(commander);

  commander.TransitImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  commander.CopyBuffer(transfer_buffer);
  commander.GenerateMipmaps();

  return image;
}

Image ObjectLoader::CreateStagingImage(const std::string& path,
                                       const VkBufferUsageFlags usage,
                                       const VkMemoryPropertyFlags properties) const {
  int image_width, image_height, image_channels;
  const std::unique_ptr<stbi_uc, void(*)(void*)> pixels(stbi_load(path.c_str(), &image_width, &image_height, &image_channels, image_settings_.stbi_format), stbi_image_free);
  if (pixels == nullptr) {
    const size_t dummy_size = image_settings_.dummy_image_extent.width * image_settings_.dummy_image_extent.height;
    const std::vector<unsigned char> dummy_colors(dummy_size, 0xff);
    return CreateStagingImageFromPixels(dummy_colors.data(), image_settings_.dummy_image_extent, usage, properties);
  }
  const VkExtent2D image_extent = { static_cast<uint32_t>(image_width), static_cast<uint32_t>(image_height) };

  return CreateStagingImageFromPixels(pixels.get(), image_extent, usage, properties);
}

std::vector<Image> ObjectLoader::CreateStagingImages(const obj::Data& data) const {
  if (!device_.GetPhysicalDevice().GetFormatFeatureSupported(image_settings_.vk_format, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    throw Error("image format does not support linear blitting");
  }
  constexpr VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                      VK_IMAGE_USAGE_SAMPLED_BIT;
  constexpr VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  std::vector<Image> images;
  images.reserve(data.mtl.size());

  for(const obj::NewMtl& mtl : data.mtl) {
    const std::string& path = mtl.map_kd;
    Image image = CreateStagingImage(path, usage, properties);
    images.emplace_back(std::move(image));
  }
  return images;
}

UniformDescriptor ObjectLoader::CreateUniformDescriptor(VkDescriptorPool descriptor_pool, const size_t frame_count) const {
  DeviceDispatchable<VkDescriptorSetLayout> descriptor_set_layout = device_.CreateUniformDescriptorSetLayout();
  const std::vector<VkDescriptorSet> descriptor_sets = device_.CreateDescriptorSets(descriptor_set_layout.GetHandle(), descriptor_pool, frame_count);

  std::vector<UniformDescriptorSet> uniform_descriptor_sets;
  uniform_descriptor_sets.reserve(frame_count);

  for(VkDescriptorSet descriptor_set : descriptor_sets) {
    Buffer buffer = device_.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Uniforms));
    device_.AllocateBuffer(buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    device_.BindBuffer(buffer);

    UniformDescriptorSet uniform_descriptor_set = {};
    uniform_descriptor_set.buffer = std::move(buffer);
    uniform_descriptor_set.handle = descriptor_set;
    uniform_descriptor_set.Update();

    uniform_descriptor_sets.emplace_back(std::move(uniform_descriptor_set));
  }
  return {
    std::move(uniform_descriptor_sets),
    std::move(descriptor_set_layout),
  };
}

[[nodiscard]] SamplerDescriptor ObjectLoader::CreateSamplerDescriptor(VkDescriptorPool descriptor_pool, std::vector<Image>&& images) const {
  DeviceDispatchable<VkDescriptorSetLayout> descriptor_set_layout =  device_.CreateSamplerDescriptorSetLayout();
  const std::vector<VkDescriptorSet> descriptor_sets = device_.CreateDescriptorSets(descriptor_set_layout.GetHandle(), descriptor_pool, images.size());
  std::vector<SamplerDescriptorSet> sampler_descriptor_sets;
  sampler_descriptor_sets.reserve(images.size());

  for(size_t i = 0; i < images.size(); ++i) {
    SamplerDescriptorSet sampler_descriptor_set = {};

    sampler_descriptor_set.sampler = device_.CreateSampler(VK_SAMPLER_MIPMAP_MODE_LINEAR, images[i].GetMipLevels());
    sampler_descriptor_set.image = std::move(images[i]);
    sampler_descriptor_set.handle = descriptor_sets[i];

    sampler_descriptor_set.Update();

    sampler_descriptor_sets.emplace_back(std::move(sampler_descriptor_set));
  }
  return {
    std::move(sampler_descriptor_sets),
    std::move(descriptor_set_layout),
  };
}

} // namespace vk