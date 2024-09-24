#include "backend/render/vk_backend.h"
#include "backend/render/vk_factory.h"
#include "backend/render/vk_config.h"
#include "backend/render/vk_types.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstring>
#include <chrono>
#include <string>
#include <limits>

namespace vk {

namespace {

class Buffer {
public:
  Buffer() = default;
  Buffer(const Buffer&) = delete;
  Buffer(Buffer&&) = default;

  Buffer(VkDevice logical_device, VkBufferUsageFlags usage, uint32_t size)
    : size_(size), logical_device_(logical_device),
      buffer_wrapper_(factory::CreateBuffer(logical_device, usage, size)) {}

  ~Buffer() = default;

  Buffer& operator=(const Buffer&) = delete;
  Buffer& operator=(Buffer&&) = default;

  void Allocate(VkPhysicalDevice physical_device, VkMemoryPropertyFlags properties) {
    memory_wrapper_ = factory::CreateBufferMemory(logical_device_, physical_device, properties, buffer_wrapper_.get());
  }

  void Bind() {
    if (const VkResult result = vkBindBufferMemory(logical_device_, buffer_wrapper_.get(), memory_wrapper_.get(), 0); result != VK_SUCCESS) {
      throw Error("failed to bind buffer memory").WithCode(result);
    }
  }

  void* Map() {
    void* data;
    if (const VkResult result = vkMapMemory(logical_device_, memory_wrapper_.get(), 0, size_, 0, &data); result != VK_SUCCESS) {
      throw Error("failed to map buffer memory").WithCode(result);
    }
    return data;
  }

  void Unmap() noexcept {
    vkUnmapMemory(logical_device_, memory_wrapper_.get());
  }

  [[nodiscard]] VkBuffer Get() const noexcept {
    return buffer_wrapper_.get();
  }
private:
  uint32_t size_;

  HandleWrapper<VkBuffer> buffer_wrapper_;
  HandleWrapper<VkDeviceMemory> memory_wrapper_;
  VkDevice logical_device_;
};

} // namespace

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

  void RecreateSwapchain();
  void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
  void UpdateUniforms(uint32_t curr_image);

  bool framebuffer_resized_;
  size_t current_frame_;

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

  Buffer vertices_buffer_, indices_buffer_;

  std::array<Buffer, config::kFrameCount> ubo_buffers_;
  std::array<void*, config::kFrameCount> ubo_mapped_;

  const std::vector<Vertex> vertices_ = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
  };

  const std::vector<Index::type> indices_ = {
    0, 1, 2, 2, 3, 0
  };
};

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

void BackendImpl::LoadModel() {
  vertices_buffer_ = CreateStagingBuffer(vertices_, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  indices_buffer_ = CreateStagingBuffer(indices_, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void BackendImpl::SetResized(bool resized) noexcept {
  framebuffer_resized_ = resized;
}

void BackendImpl::UpdateUniforms(uint32_t curr_image) {
  static const std::chrono::time_point start_time = std::chrono::high_resolution_clock::now();

  const std::chrono::time_point curr_time = std::chrono::high_resolution_clock::now();
  const float time = std::chrono::duration<float>(curr_time - start_time).count();

  UniformBufferObject ubo = {};
  ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.proj = glm::perspective(glm::radians(45.0f), swapchain_details_.extent.width / static_cast<float>(swapchain_details_.extent.height), 0.1f, 10.0f);
  ubo.proj[1][1] *= -1;

  std::memcpy(ubo_mapped_[curr_image], &ubo, sizeof(ubo));
}

BackendImpl::BackendImpl(GLFWwindow* window)
    : framebuffer_resized_(false), current_frame_(), window_(window) {
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

    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = ubo_buffers_[i].Get();
    buffer_info.offset = 0;
    buffer_info.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptor_sets_[i];
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &buffer_info;

    vkUpdateDescriptorSets(logical_device, 1, &descriptor_write, 0, nullptr);
  }
  render_pass_wrapper_ = factory::CreateRenderPass(logical_device, swapchain_details_.format);
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
  framebuffers_wrapped_ = factory::CreateFramebuffers(logical_device, image_views_wrapped_, render_pass, swapchain_details_.extent);

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
  framebuffers_wrapped_ = factory::CreateFramebuffers(logical_device, image_views_wrapped_, render_pass, swapchain_details_.extent);
}

void BackendImpl::Render() {
  uint32_t image_idx;

  VkDevice logical_device = logical_device_wrapper_.get();

  VkFence fence = fences_wrapped_[current_frame_].get();
  VkSemaphore image_semaphore = image_semaphores_wrapped_[current_frame_].get();
  VkSemaphore render_semaphore = render_semaphores_wrapped_[current_frame_].get();

  VkSwapchainKHR swapchain = swapchain_wrapper_.get();
  VkRenderPass render_pass = render_pass_wrapper_.get();
  VkCommandBuffer cmd_buffer = cmd_buffers_[current_frame_];
  VkPipelineLayout pipeline_layout = pipeline_layout_wrapper_.get();
  VkPipeline pipeline = pipeline_wrapper_.get();

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
  UpdateUniforms(current_frame_);
  if (const VkResult result = vkResetFences(logical_device, 1, &fence); result != VK_SUCCESS) {
    throw Error("failed to reset fences").WithCode(result);
  }
  if (const VkResult result = vkResetCommandBuffer(cmd_buffer, 0); result != VK_SUCCESS) {
    throw Error("failed to reset command buffer").WithCode(result);
  }
  VkCommandBufferBeginInfo cmd_buffer_begin_info = {};
  cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  if (const VkResult result = vkBeginCommandBuffer(cmd_buffer, &cmd_buffer_begin_info); result != VK_SUCCESS) {
    throw Error("failed to begin recording command buffer").WithCode(result);
  }
  VkClearValue clear_color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  VkRenderPassBeginInfo render_pass_begin_info = {};
  render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_begin_info.renderPass = render_pass;
  render_pass_begin_info.framebuffer = framebuffers_wrapped_[image_idx].get();
  render_pass_begin_info.renderArea.offset = {0, 0};
  render_pass_begin_info.renderArea.extent = swapchain_details_.extent;
  render_pass_begin_info.clearValueCount = 1;
  render_pass_begin_info.pClearValues = &clear_color;
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
  VkDeviceSize offsets[] = {0};

  vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &vertices_buffer, offsets);
  vkCmdBindIndexBuffer(cmd_buffer, indices_buffer, 0, Index::type_enum);
  vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_sets_[current_frame_], 0, nullptr);
  vkCmdDrawIndexed(cmd_buffer, static_cast<uint32_t>(indices_.size()), 1, 0, 0, 0);

  vkCmdEndRenderPass(cmd_buffer);
  if (const VkResult result = vkEndCommandBuffer(cmd_buffer); result != VK_SUCCESS) {
    throw Error("failed to record command buffer").WithCode(result);
  }
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
  current_frame_ = (current_frame_ + 1) % config::kFrameCount;
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