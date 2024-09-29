#include "backend/render/vk_backend.h"
#include "backend/render/vk_factory.h"
#include "backend/render/vk_config.h"
#include "backend/render/vk_types.h"
#include "backend/render/vk_wrappers.h"
#include "obj/parser.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <cstring>
#include <chrono>
#include <iostream>
#include <string>
#include <map>
#include <limits>

namespace vk {

namespace {

void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool cmd_pool, VkDevice logical_device, VkQueue graphics_queue) {
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphics_queue);

  vkFreeCommandBuffers(logical_device, cmd_pool, 1, &commandBuffer);
}

VkCommandBuffer BeginSingleTimeCommands(VkDevice logical_device, VkCommandPool cmd_pool) {
  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandPool = cmd_pool;
  alloc_info.commandBufferCount = 1;

  VkCommandBuffer command_buffer;
  vkAllocateCommandBuffers(logical_device, &alloc_info, &command_buffer);

  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(command_buffer, &begin_info);

  return command_buffer;
}

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

void UpdateTextureDescriptorSets(VkDevice logical_device, VkImageView image_view, VkSampler texture_sampler, VkDescriptorSet descriptor_set) {
  VkDescriptorImageInfo image_info = {};
  image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  image_info.imageView = image_view;
  image_info.sampler = texture_sampler;

  VkWriteDescriptorSet descriptor_write = {};

  descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor_write.dstSet = descriptor_set;
  descriptor_write.dstBinding = 1;
  descriptor_write.dstArrayElement = 0;
  descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptor_write.descriptorCount = 1;
  descriptor_write.pImageInfo = &image_info;

  vkUpdateDescriptorSets(logical_device, 1, &descriptor_write, 0, nullptr);
}

VkFormat FindSupportedFormat(const std::vector<VkFormat>& formats, VkPhysicalDevice physical_device, VkImageTiling tiling, VkFormatFeatureFlags features) {
  for (const VkFormat format : formats) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);
    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    }
    if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw Error("failed to find supported format");
}

inline VkFormat FindDepthFormat(VkPhysicalDevice physical_device) {
  return FindSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      physical_device,
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
  );
}

inline Image CreateDepthImage(VkDevice logical_device, VkPhysicalDevice physical_device, VkExtent2D extent) {
  VkFormat depth_format = FindDepthFormat(physical_device);

  Image depth_image(logical_device, extent.width, extent.height, STBI_rgb_alpha, depth_format,  VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
  depth_image.Allocate(physical_device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  depth_image.Bind();
  depth_image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);

  return depth_image;
}

} // namespace

struct Mesh {
  std::vector<Index::type> indices;
  std::vector<Vertex> vertices;
  std::vector<Image> textures;
  std::vector<obj::UseMtl> usemtl;
};

class BackendImpl {
 public:
  explicit BackendImpl(GLFWwindow* window);
  ~BackendImpl();

  void Render();
  void LoadModel();
  void SetResized(bool resized) noexcept;
 private:
  template<typename Tp>
  Buffer CreateStagingBuffer(const std::vector<Tp>& data, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
  Image CreateStagingImage(const std::string& path, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

  void RecreateSwapchain();
  void UpdateUniforms();
  void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
  void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
  void RecordCommandBuffer(VkCommandBuffer cmd_buffer, size_t image_idx);
  void TransitionImageLayout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);

  bool framebuffer_resized_;
  size_t curr_frame_;

  GLFWwindow* window_;

  HandleWrapper<VkInstance> instance_wrapper_;
#ifdef DEBUG
  HandleWrapper<VkDebugUtilsMessengerEXT> messenger_wrapper_;
#endif // DEBUG
  HandleWrapper<VkSurfaceKHR> surface_wrapper_;

  VkPhysicalDevice physical_device_;
  QueueFamilyIndices family_indices_;
  HandleWrapper<VkDevice> logical_device_wrapper_;
  VkQueue graphics_queue_, present_queue_;

  HandleWrapper<VkSwapchainKHR> swapchain_wrapper_;
  SwapchainDetails swapchain_details_;
  std::vector<VkImage> swapchain_images_;
  std::vector<HandleWrapper<VkImageView>> image_views_wrapped_;
  std::vector<HandleWrapper<VkFramebuffer>> framebuffers_wrapped_;

  HandleWrapper<VkDescriptorSetLayout> descriptor_set_layout_wrapper_;
  HandleWrapper<VkDescriptorPool> descriptor_pool_wrapper_;
  std::vector<VkDescriptorSet> descriptor_sets_;

  HandleWrapper<VkRenderPass> render_pass_wrapper_;
  HandleWrapper<VkPipelineLayout> pipeline_layout_wrapper_;
  HandleWrapper<VkPipeline> pipeline_wrapper_;

  HandleWrapper<VkCommandPool> cmd_pool_wrapper_;
  std::vector<VkCommandBuffer> cmd_buffers_;

  std::vector<HandleWrapper<VkSemaphore>> image_semaphores_wrapped_;
  std::vector<HandleWrapper<VkSemaphore>> render_semaphores_wrapped_;
  std::vector<HandleWrapper<VkFence>> fences_wrapped_;

  HandleWrapper<VkSampler> texture_sampler_;

  Buffer vertices_buffer_, indices_buffer_;
  Image depth_image_;

  std::array<Buffer, config::kFrameCount> ubo_buffers_;
  std::array<void*, config::kFrameCount> ubo_mapped_;

  Mesh mesh_;
};

BackendImpl::BackendImpl(GLFWwindow* window)
  : framebuffer_resized_(false), curr_frame_(), window_(window) {
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width[[maybe_unused]], int height[[maybe_unused]]) {
    auto impl = static_cast<BackendImpl*>(glfwGetWindowUserPointer(window));
    impl->SetResized(true);
  });

  instance_wrapper_ = factory::CreateInstance();
  VkInstance instance = instance_wrapper_.get();

#ifdef DEBUG
  messenger_wrapper_ = factory::CreateMessenger(instance);
#endif // DEBUG
  surface_wrapper_ = factory::CreateSurface(instance, window_);
  VkSurfaceKHR surface = surface_wrapper_.get();

  std::tie(physical_device_, family_indices_) = factory::CreatePhysicalDevice(instance, surface);
  logical_device_wrapper_ = factory::CreateLogicalDevice(physical_device_, family_indices_);
  VkDevice logical_device = logical_device_wrapper_.get();

  vkGetDeviceQueue(logical_device, family_indices_.graphic, 0, &graphics_queue_);
  vkGetDeviceQueue(logical_device, family_indices_.present, 0, &present_queue_);

  std::tie(swapchain_wrapper_, swapchain_details_) = factory::CreateSwapchain(window_, surface, physical_device_, family_indices_, logical_device);
  VkSwapchainKHR swapchain = swapchain_wrapper_.get();
  swapchain_images_ = factory::CreateSwapchainImages(swapchain, logical_device);

  descriptor_set_layout_wrapper_ = factory::CreateDescriptorSetLayout(logical_device);
  VkDescriptorSetLayout descriptor_set_layout = descriptor_set_layout_wrapper_.get();

  descriptor_pool_wrapper_ = factory::CreateDescriptorPool(logical_device, config::kFrameCount);
  VkDescriptorPool descriptor_pool = descriptor_pool_wrapper_.get();

  descriptor_sets_ = factory::CreateDescriptorSets(logical_device, descriptor_set_layout, descriptor_pool, config::kFrameCount);

  for(size_t i = 0; i < ubo_buffers_.size(); ++i) {
    ubo_buffers_[i] = Buffer(logical_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(UniformBufferObject));
    ubo_buffers_[i].Allocate(physical_device_, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    ubo_buffers_[i].Bind();

    ubo_mapped_[i] = ubo_buffers_[i].Map();
    UpdateBufferDescriptorSets(logical_device, ubo_buffers_[i].Get(), descriptor_sets_[i]);
  }
  depth_image_ = CreateDepthImage(logical_device, physical_device_, swapchain_details_.extent);

  render_pass_wrapper_ = factory::CreateRenderPass(logical_device, swapchain_details_.format, depth_image_.GetFormat());
  VkRenderPass render_pass = render_pass_wrapper_.get();

  pipeline_layout_wrapper_ = factory::CreatePipelineLayout(logical_device, descriptor_set_layout);
  VkPipelineLayout pipeline_layout = pipeline_layout_wrapper_.get();
  pipeline_wrapper_ = factory::CreatePipeline(logical_device, pipeline_layout, render_pass,
  {
      {
          VK_SHADER_STAGE_VERTEX_BIT,
          factory::CreateShaderModule(logical_device, "shaders/vert.spv"),
          "main"
      },
      {
          VK_SHADER_STAGE_FRAGMENT_BIT,
          factory::CreateShaderModule(logical_device, "shaders/frag.spv"),
          "main"
      }
    }
  );
  image_views_wrapped_ = factory::CreateImageViews(swapchain_images_, logical_device, swapchain_details_.format);
  framebuffers_wrapped_ = factory::CreateFramebuffers(logical_device, image_views_wrapped_, depth_image_.GetView(), render_pass, swapchain_details_.extent);

  cmd_pool_wrapper_ = factory::CreateCommandPool(logical_device, family_indices_);
  cmd_buffers_ = factory::CreateCommandBuffers(logical_device, cmd_pool_wrapper_.get(), config::kFrameCount);

  image_semaphores_wrapped_.reserve(config::kFrameCount);
  render_semaphores_wrapped_.reserve(config::kFrameCount);
  fences_wrapped_.reserve(config::kFrameCount);
  for(size_t i = 0; i < config::kFrameCount; ++i) {
    image_semaphores_wrapped_.emplace_back(factory::CreateSemaphore(logical_device));
    render_semaphores_wrapped_.emplace_back(factory::CreateSemaphore(logical_device));
    fences_wrapped_.emplace_back(factory::CreateFence(logical_device));
  }
  texture_sampler_ = factory::CreateTextureSampler(logical_device, physical_device_);
}

template<typename Tp>
Buffer BackendImpl::CreateStagingBuffer(const std::vector<Tp>& data, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
  uint32_t data_size = sizeof(Tp) * data.size();
  VkDevice logical_device = logical_device_wrapper_.get();

  Buffer transfer_buffer(logical_device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, data_size);
  transfer_buffer.Allocate(physical_device_, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  transfer_buffer.Bind();
  {
    void* mapped_buffer = transfer_buffer.Map();
    std::memcpy(mapped_buffer, data.data(), data_size);
    transfer_buffer.Unmap();
  }
  Buffer staging_buffer(logical_device, usage, data_size);
  staging_buffer.Allocate(physical_device_, properties);
  staging_buffer.Bind();

  CopyBuffer(transfer_buffer.Get(), staging_buffer.Get(), data_size);

  return staging_buffer;
}

Image BackendImpl::CreateStagingImage(const std::string& path, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
  stbi_set_flip_vertically_on_load(true);

  int image_width, image_height, image_channels;
  stbi_uc* pixels = stbi_load(path.c_str(), &image_width, &image_height, &image_channels, STBI_rgb_alpha);
  if (pixels == nullptr) {
    throw Error("failed to load texture image");
  }
  VkDeviceSize image_size = image_width * image_height * STBI_rgb_alpha;
  HandleWrapper<stbi_uc*> image_wrapper(pixels, [](stbi_uc* pixels) {
    stbi_image_free(pixels);
  });
  VkDevice logical_device = logical_device_wrapper_.get();
  Buffer transfer_buffer(logical_device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, image_size);
  transfer_buffer.Allocate(physical_device_, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  transfer_buffer.Bind();
  {
    void* mapped_buffer = transfer_buffer.Map();
    std::memcpy(mapped_buffer, pixels, static_cast<size_t>(image_size));
    transfer_buffer.Unmap();
  }
  VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;
  Image image_wrapped(logical_device, image_width, image_height, STBI_rgb_alpha, image_format, VK_IMAGE_TILING_OPTIMAL, usage);
  image_wrapped.Allocate(physical_device_, properties);
  image_wrapped.Bind();

  VkImage image = image_wrapped.Get();

  TransitionImageLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  CopyBufferToImage(transfer_buffer.Get(), image, static_cast<uint32_t>(image_width), static_cast<uint32_t>(image_height));
  TransitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  image_wrapped.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);

  return image_wrapped;
}

struct compare {
  bool operator()(const obj::Index& lhs, const obj::Index& rhs) const noexcept {
    if (lhs.fv < rhs.fv) return true;
    if (rhs.fv < lhs.fv) return false;
    if (lhs.fn < rhs.fn) return true;
    if (rhs.fn < lhs.fn) return false;
    if (lhs.ft < rhs.ft) return true;
    return rhs.ft < lhs.ft;
  }
};

Mesh FromDataToMesh(obj::Data& data) {
  Mesh mesh;
  std::map<obj::Index, unsigned int, compare> index_map;

  mesh.vertices.reserve(data.indices.size());
  mesh.indices.reserve(data.indices.size());

  mesh.usemtl = std::move(data.usemtl);

  unsigned int next_combined_idx = 0, combined_idx = 0;
  for (const obj::Index& index : data.indices) {
    if (index_map.count(index)) {
      combined_idx = index_map.at(index);
    } else {
      combined_idx = next_combined_idx;
      index_map.insert({index, combined_idx});
      unsigned int i_v = index.fv * 3, i_n = index.fn * 3, i_t = index.ft * 2;
      Vertex vertex = {
        {data.v[i_v], data.v[i_v + 1], data.v[i_v + 2]},
        {data.vn[i_n], data.vn[i_n + 1], data.vn[i_n + 2]},
        {data.vt[i_t], data.vt[i_t + 1]}
      };
      mesh.vertices.push_back(vertex);
      ++next_combined_idx;
    }
    mesh.indices.push_back(combined_idx);
  }
  return mesh;
}

void BackendImpl::LoadModel() {
  obj::Data data = obj::ParseFromFile("/mnt/c/Users/user/CLionProjects/VulkanEngine/obj/gnom/rizhignom.obj");

  mesh_ = FromDataToMesh(data);
  for(const obj::NewMtl& mtl : data.mtl) {
    try {
      Image texture = CreateStagingImage(mtl.map_kd, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      mesh_.textures.emplace_back(std::move(texture));
    } catch (...) {

    }
  }
  vertices_buffer_ = CreateStagingBuffer(mesh_.vertices, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  indices_buffer_ = CreateStagingBuffer(mesh_.indices, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void BackendImpl::SetResized(bool resized) noexcept {
  framebuffer_resized_ = resized;
}

void BackendImpl::UpdateUniforms() {
  static const std::chrono::time_point start_time = std::chrono::high_resolution_clock::now();

  const std::chrono::time_point curr_time = std::chrono::high_resolution_clock::now();
  const float time = std::chrono::duration<float>(curr_time - start_time).count();

  UniformBufferObject ubo = {};
  ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.proj = glm::perspective(glm::radians(45.0f), swapchain_details_.extent.width / static_cast<float>(swapchain_details_.extent.height), 0.1f, 10.0f);
  ubo.proj[1][1] *= -1;

  std::memcpy(ubo_mapped_[curr_frame_], &ubo, sizeof(ubo));
}

void BackendImpl::CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) {
  VkCommandPool cmd_pool = cmd_pool_wrapper_.get();
  VkDevice logical_device = logical_device_wrapper_.get();

  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandPool = cmd_pool;
  alloc_info.commandBufferCount = 1;

  VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;
  if (const VkResult result = vkAllocateCommandBuffers(logical_device, &alloc_info, &cmd_buffer); result != VK_SUCCESS) {
    throw Error("failed allocate command buffer").WithCode(result);
  }
  HandleWrapper<VkCommandBuffer> cmd_buffer_wrapper(
    cmd_buffer,
    [logical_device, cmd_pool](VkCommandBuffer cmd_buffer) {
    vkFreeCommandBuffers(logical_device, cmd_pool, 1, &cmd_buffer);
  });

  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  if (const VkResult result = vkBeginCommandBuffer(cmd_buffer, &begin_info); result != VK_SUCCESS) {
    throw Error("failed to begin recording command buffer").WithCode(result);
  }

  VkBufferCopy copy_region = {};
  copy_region.size = size;
  vkCmdCopyBuffer(cmd_buffer, src, dst, 1, &copy_region);

  if (const VkResult result = vkEndCommandBuffer(cmd_buffer); result != VK_SUCCESS) {
    throw Error("failed to record command buffer").WithCode(result);
  }
  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd_buffer;

  if (const VkResult result = vkQueueSubmit(graphics_queue_, 1, &submit_info, VK_NULL_HANDLE); result != VK_SUCCESS) {
    throw Error("failed to submit draw command buffer").WithCode(result);
  }
  if (const VkResult result = vkQueueWaitIdle(graphics_queue_); result != VK_SUCCESS) {
    throw Error("failed to queue wait idle").WithCode(result);
  }
}

void BackendImpl::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
  VkDevice logical_device = logical_device_wrapper_.get();
  VkCommandPool cmd_pool = cmd_pool_wrapper_.get();

  VkCommandBuffer command_buffer = BeginSingleTimeCommands(logical_device, cmd_pool);

  VkBufferImageCopy region = {};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = { width, height, 1 };

  vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  EndSingleTimeCommands(command_buffer, cmd_pool, logical_device, graphics_queue_);
}

void BackendImpl::TransitionImageLayout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout) {
  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags source_stage;
  VkPipelineStageFlags destination_stage;

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    throw Error("unsupported layout transition");
  }

  VkDevice logical_device = logical_device_wrapper_.get();
  VkCommandPool cmd_pool = cmd_pool_wrapper_.get();

  VkCommandBuffer command_buffer = BeginSingleTimeCommands(logical_device, cmd_pool);
  vkCmdPipelineBarrier(
      command_buffer,
      source_stage, destination_stage,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier
  );
  EndSingleTimeCommands(command_buffer, cmd_pool, logical_device, graphics_queue_);
}

void BackendImpl::RecreateSwapchain() {
  int width = 0, height = 0;
  glfwGetFramebufferSize(window_, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window_, &width, &height);
    glfwWaitEvents();
  }
  VkDevice logical_device = logical_device_wrapper_.get();
  VkRenderPass render_pass = render_pass_wrapper_.get();
  VkSurfaceKHR surface = surface_wrapper_.get();

  if (const VkResult result = vkDeviceWaitIdle(logical_device); result != VK_SUCCESS) {
    throw Error("failed to idle device");
  }

  framebuffers_wrapped_.clear();
  image_views_wrapped_.clear();
  swapchain_wrapper_.reset();

  std::tie(swapchain_wrapper_, swapchain_details_) = factory::CreateSwapchain(window_, surface, physical_device_, family_indices_, logical_device);
  swapchain_images_ = factory::CreateSwapchainImages(swapchain_wrapper_.get(), logical_device);

  image_views_wrapped_ = factory::CreateImageViews(swapchain_images_, logical_device, swapchain_details_.format);

  depth_image_ = CreateDepthImage(logical_device, physical_device_, swapchain_details_.extent);

  framebuffers_wrapped_ = factory::CreateFramebuffers(logical_device, image_views_wrapped_, depth_image_.GetView(), render_pass, swapchain_details_.extent);
}

void BackendImpl::RecordCommandBuffer(VkCommandBuffer cmd_buffer, size_t image_idx) {
  VkRenderPass render_pass = render_pass_wrapper_.get();
  VkPipelineLayout pipeline_layout = pipeline_layout_wrapper_.get();
  VkPipeline pipeline = pipeline_wrapper_.get();

  VkCommandBufferBeginInfo cmd_buffer_begin_info = {};
  cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  if (const VkResult result = vkBeginCommandBuffer(cmd_buffer, &cmd_buffer_begin_info); result != VK_SUCCESS) {
    throw Error("failed to begin recording command buffer").WithCode(result);
  }
  std::array<VkClearValue, 2> clear_values = {};
  clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  clear_values[1].depthStencil = {1.0f, 0};

  VkRenderPassBeginInfo render_pass_begin_info = {};
  render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_begin_info.renderPass = render_pass;
  render_pass_begin_info.framebuffer = framebuffers_wrapped_[image_idx].get();
  render_pass_begin_info.renderArea.offset = {0, 0};
  render_pass_begin_info.renderArea.extent = swapchain_details_.extent;
  render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
  render_pass_begin_info.pClearValues = clear_values.data();
  vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(swapchain_details_.extent.width);
  viewport.height = static_cast<float>(swapchain_details_.extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

  VkRect2D scissor = {};
  scissor.offset = {0, 0};
  scissor.extent = swapchain_details_.extent;
  vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

  VkBuffer vertices_buffer = vertices_buffer_.Get();
  VkBuffer indices_buffer = indices_buffer_.Get();
  VkDescriptorSet descriptor_set = descriptor_sets_[curr_frame_];

  VkDeviceSize prev_offset = 0;
  VkDeviceSize vertex_offsets[] = {0};

  for(const obj::UseMtl& usemtl : mesh_.usemtl) {
    VkDeviceSize curr_offset = prev_offset * sizeof(Index::type);

    UpdateTextureDescriptorSets(logical_device_wrapper_.get(), mesh_.textures[usemtl.index].GetView(), texture_sampler_.get(), descriptor_set);
    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &vertices_buffer, vertex_offsets);
    vkCmdBindIndexBuffer(cmd_buffer, indices_buffer, curr_offset, Index::type_enum);
    vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);
    vkCmdDrawIndexed(cmd_buffer, static_cast<uint32_t>(usemtl.offset - prev_offset), 1, 0, 0, 0);

    prev_offset = usemtl.offset;
  }
  vkCmdEndRenderPass(cmd_buffer);
  if (const VkResult result = vkEndCommandBuffer(cmd_buffer); result != VK_SUCCESS) {
    throw Error("failed to record command buffer").WithCode(result);
  }
}

void BackendImpl::Render() {
  uint32_t image_idx;

  VkDevice logical_device = logical_device_wrapper_.get();

  VkFence fence = fences_wrapped_[curr_frame_].get();
  VkSemaphore image_semaphore = image_semaphores_wrapped_[curr_frame_].get();
  VkSemaphore render_semaphore = render_semaphores_wrapped_[curr_frame_].get();

  VkSwapchainKHR swapchain = swapchain_wrapper_.get();

  VkCommandBuffer cmd_buffer = cmd_buffers_[curr_frame_];

  if (const VkResult result = vkWaitForFences(logical_device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()); result != VK_SUCCESS) {
    throw Error("failed to wait for fences").WithCode(result);
  }
  if (const VkResult result = vkAcquireNextImageKHR(logical_device, swapchain, std::numeric_limits<uint64_t>::max(), image_semaphore, VK_NULL_HANDLE, &image_idx); result != VK_SUCCESS) {
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      RecreateSwapchain();
      return;
    }
    if (result != VK_SUBOPTIMAL_KHR) {
      throw Error("failed to acquire next image").WithCode(result);
    }
  }
  UpdateUniforms();
  if (const VkResult result = vkResetFences(logical_device, 1, &fence); result != VK_SUCCESS) {
    throw Error("failed to reset fences").WithCode(result);
  }
  if (const VkResult result = vkResetCommandBuffer(cmd_buffer, 0); result != VK_SUCCESS) {
    throw Error("failed to reset command buffer").WithCode(result);
  }
  RecordCommandBuffer(cmd_buffer, image_idx);

  std::vector<VkPipelineStageFlags> pipeline_stage_flags = config::GetPipelineStageFlags();

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &image_semaphore;
  submit_info.pWaitDstStageMask = pipeline_stage_flags.data();
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd_buffer;
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &render_semaphore;

  if (const VkResult result = vkQueueSubmit(graphics_queue_, 1, &submit_info, fence); result != VK_SUCCESS) {
    throw Error("failed to submit draw command buffer").WithCode(result);
  }

  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &render_semaphore;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &swapchain;
  present_info.pImageIndices = &image_idx;

  const VkResult result = vkQueuePresentKHR(present_queue_, &present_info);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebuffer_resized_) {
    framebuffer_resized_ = false;
    RecreateSwapchain();
  } else if (result != VK_SUCCESS) {
    throw Error("failed to queue present").WithCode(result);
  }
  curr_frame_ = (curr_frame_ + 1) % config::kFrameCount;
}

BackendImpl::~BackendImpl() {
  vkDeviceWaitIdle(logical_device_wrapper_.get());
}

Backend::Backend(GLFWwindow* window)
  : impl_(new BackendImpl(window))  {}

Backend::~Backend() { delete impl_; }

void Backend::Render() const {
  impl_->Render();
}

void Backend::LoadModel() {
  impl_->LoadModel();
}


} // vk