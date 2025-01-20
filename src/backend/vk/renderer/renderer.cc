#include "backend/vk/renderer/renderer.h"

#include <array>
#include <chrono>
#include <cstring>
#include <limits>

#include "backend/vk/renderer/device_selector.h"
#include "backend/vk/renderer/device_dispatchable_factory.h"
#include "backend/vk/renderer/instance_dispatchable_factory.h"
#include "backend/vk/renderer/error.h"
#include "backend/vk/renderer/object_loader.h"
#include "backend/vk/renderer/shader.h"

namespace vk {

namespace {

template<typename T, typename ...Args>
std::vector<T> MergeVectors(const Args&... args) {
  std::vector<T> v1;
  (v1.insert(v1.end(), args.begin(), args.end()), ...);
  return v1;
}

} // namespace

Renderer::Renderer(Config config, Window& window)
  : config_(std::move(config)),
    window_(window),
    framebuffer_resized_(false),
    curr_frame_(0),
    instance_(config_.app_info, MergeVectors<const char*>(window.GetExtensions(),
                                                                   config_.instance_extensions)) {
  window.SetWindowUserPointer(this);
  window.SetWindowResizedCallback([](void* user_ptr, [[maybe_unused]] int width, [[maybe_unused]] int height) {
    auto render = static_cast<Renderer*>(user_ptr);
    render->framebuffer_resized_ = true;
  });

  const InstanceDispatchableFactory instance_dispatchable_factory(instance_);

#ifdef DEBUG
  messenger_ = instance_dispatchable_factory.CreateMessenger();
#endif

  surface_ = instance_dispatchable_factory.CreateSurface(window);

  config_.device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  config_.device_extensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
#ifdef __APPLE__
  config_.device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

  DeviceSelector::Requirements requirements = {};
  requirements.present = true;
  requirements.graphic = true;
  requirements.anisotropy = true;
  requirements.surface = surface_.GetHandle();
  requirements.extensions = config_.device_extensions;

  const std::vector<VkPhysicalDevice> devices = instance_.EnumeratePhysicalDevices();

  if (std::optional<Device> device = DeviceSelector(devices).Select(requirements); !device) {
    throw Error("failed to find suitable device");
  } else {
    device_ = std::move(*device);
  }

  const DeviceDispatchableFactory device_dispatchable_factory(device_);

  CreateSwapchain(device_dispatchable_factory);
  render_pass_ = device_dispatchable_factory.CreateRenderPass(swapchain_.GetFormat(),  depth_image_.GetFormat());
  CreateFramebuffers(device_dispatchable_factory);

  cmd_pool_ = device_dispatchable_factory.CreateCommandPool();
  cmd_buffers_ = device_dispatchable_factory.CreateCommandBuffers(cmd_pool_.GetHandle(), config_.frame_count);
}

void Renderer::LoadModel(const std::string& path) {
  object_ = ObjectLoader(device_,
                         config_.image_settings,
                         cmd_pool_.GetHandle())
                        .Load(path, config_.frame_count);

  const std::vector descriptor_set_layouts = { object_.ubo.descriptor_set_layout.GetHandle(), object_.tbo.descriptor_set_layout.GetHandle() };

  const DeviceDispatchableFactory device_factory(device_);

  pipeline_layout_ = device_factory.CreatePipelineLayout(descriptor_set_layouts);

  const std::vector shaders = GetShaders();

  std::vector<ShaderModule> shader_modules;
  shader_modules.reserve(shaders.size());
  for(const ShaderInfo& shader : shaders) {
    shader_modules.emplace_back(device_factory.CreateShaderModule(shader));
  }
  pipeline_ = device_factory.CreatePipeline(pipeline_layout_.GetHandle(), render_pass_.GetHandle(), Vertex::GetAttributeDescriptions(), Vertex::GetBindingDescriptions(), shader_modules);

  uniforms_buff_.reserve(object_.ubo.buffers.size());
  for(const auto& buffer : object_.ubo.buffers) {
    auto uniforms = static_cast<Uniforms*>(buffer.Map());
    uniforms_buff_.emplace_back(uniforms);
  }
}

void Renderer::CreateSwapchain(const DeviceDispatchableFactory& dispatchable_factory) {
  VkExtent2D swapchain_extent = {};
  swapchain_extent.width = window_.GetWidth();
  swapchain_extent.height = window_.GetHeight();

  swapchain_ = dispatchable_factory.CreateSwapchain(swapchain_extent, surface_.GetHandle());

  depth_image_ = dispatchable_factory.CreateImage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                  VK_IMAGE_ASPECT_DEPTH_BIT,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                  swapchain_.GetExtent(),
                                                  device_.FindDepthFormat(),
                                                  VK_IMAGE_TILING_OPTIMAL);
}


void Renderer::CreateFramebuffers(const DeviceDispatchableFactory& dispatchable_factory) {
  const std::vector<VkImage> images = swapchain_.GetImages();
  image_views_.reserve(images.size());
  framebuffers_.reserve(images.size());

  for(VkImage image : images) {
    DeviceDispatchable<VkImageView> image_view = dispatchable_factory.CreateImageView(image, VK_IMAGE_ASPECT_COLOR_BIT, swapchain_.GetFormat());
    DeviceDispatchable<VkFramebuffer> framebuffer = dispatchable_factory.CreateFramebuffer({image_view.GetHandle(), depth_image_.GetView()}, render_pass_.GetHandle(), swapchain_.GetExtent());

    image_views_.emplace_back(std::move(image_view));
    framebuffers_.emplace_back(std::move(framebuffer));
  }
  const size_t count = config_.frame_count;

  image_semaphores_.reserve(count);
  render_semaphores_.reserve(count);
  fences_.reserve(count);

  for(size_t i = 0; i < count; ++i) {
    image_semaphores_.emplace_back(dispatchable_factory.CreateSemaphore());
    render_semaphores_.emplace_back(dispatchable_factory.CreateSemaphore());
    fences_.emplace_back(dispatchable_factory.CreateFence());
  }
}

void Renderer::RecreateSwapchain() {
  window_.WaitUntilResized();

  if (const VkResult result = vkDeviceWaitIdle(device_.GetLogical()); result != VK_SUCCESS) {
    throw Error("failed to idle device");
  }
  image_semaphores_.clear();
  render_semaphores_.clear();
  fences_.clear();

  framebuffers_.clear();
  image_views_.clear();
  swapchain_ = Swapchain();

  const DeviceDispatchableFactory dispatchable_factory(device_);

  CreateSwapchain(dispatchable_factory);
  CreateFramebuffers(dispatchable_factory);
}

void Renderer::RecordCommandBuffer(VkCommandBuffer cmd_buffer, const size_t image_idx) {
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
  render_pass_begin_info.renderPass = render_pass_.GetHandle();
  render_pass_begin_info.framebuffer = framebuffers_[image_idx].GetHandle();
  render_pass_begin_info.renderArea.offset = {0, 0};
  render_pass_begin_info.renderArea.extent = swapchain_.GetExtent();
  render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
  render_pass_begin_info.pClearValues = clear_values.data();
  vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_.GetHandle());

  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = static_cast<float>(swapchain_.GetExtent().height);
  viewport.width = static_cast<float>(swapchain_.GetExtent().width);
  viewport.height = -static_cast<float>(swapchain_.GetExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

  VkRect2D scissor = {};
  scissor.offset = {0, 0};
  scissor.extent = swapchain_.GetExtent();
  vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

  VkBuffer vertices_buffer = object_.vertices.GetHandle();
  VkBuffer indices_buffer = object_.indices.GetHandle();

  VkDeviceSize prev_offset = 0;
  std::array<VkDeviceSize, 1> vertex_offsets = {};

  for(const auto[index, offset] : object_.usemtl) {
    const VkDeviceSize curr_offset = prev_offset * sizeof(Index);

    vkCmdBindVertexBuffers(cmd_buffer, 0, vertex_offsets.size(), &vertices_buffer, vertex_offsets.data());
    vkCmdBindIndexBuffer(cmd_buffer, indices_buffer, curr_offset, IndexType<Index>::value);
    vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_.GetHandle(), 0, 1, &object_.ubo.descriptor_sets[curr_frame_], 0, nullptr);
    vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_.GetHandle(), 1, 1, &object_.tbo.descriptor_sets[index], 0, nullptr);

    vkCmdDrawIndexed(cmd_buffer, static_cast<uint32_t>(offset - prev_offset), 1, 0, 0, 0);

    prev_offset = offset;
  }
  vkCmdEndRenderPass(cmd_buffer);
  if (const VkResult result = vkEndCommandBuffer(cmd_buffer); result != VK_SUCCESS) {
    throw Error("failed to record command buffer").WithCode(result);
  }
}

void Renderer::UpdateUniforms() const {
  const engine::Uniforms& uniforms = model_.GetUniforms();
  std::memcpy(uniforms_buff_[curr_frame_], &uniforms, sizeof(Uniforms));
}

void Renderer::RenderFrame() {
  uint32_t image_idx;

  VkQueue graphics_queue = device_.GetGraphicsQueue();
  VkQueue present_queue = device_.GetPresentQueue();

  VkFence fence = fences_[curr_frame_].GetHandle();
  VkSemaphore image_semaphore = image_semaphores_[curr_frame_].GetHandle();
  VkSemaphore render_semaphore = render_semaphores_[curr_frame_].GetHandle();

  VkCommandBuffer cmd_buffer = cmd_buffers_[curr_frame_];
  VkSwapchainKHR swapchain = swapchain_.GetHandle();

  if (const VkResult result = vkWaitForFences(device_.GetLogical(), 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()); result != VK_SUCCESS) {
    throw Error("failed to wait for fences").WithCode(result);
  }
  if (const VkResult result = vkAcquireNextImageKHR(device_.GetLogical(), swapchain_.GetHandle(), std::numeric_limits<uint64_t>::max(), image_semaphore, VK_NULL_HANDLE, &image_idx); result != VK_SUCCESS) {
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
      RecreateSwapchain();
      return;
    }
    throw Error("failed to acquire next image").WithCode(result);
  }
  UpdateUniforms();
  if (const VkResult result = vkResetFences(device_.GetLogical(), 1, &fence); result != VK_SUCCESS) {
    throw Error("failed to reset fences").WithCode(result);
  }
  if (const VkResult result = vkResetCommandBuffer(cmd_buffer, 0); result != VK_SUCCESS) {
    throw Error("failed to reset command buffer").WithCode(result);
  }
  RecordCommandBuffer(cmd_buffer, image_idx);

  const std::vector<VkPipelineStageFlags> pipeline_stages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &image_semaphore;
  submit_info.pWaitDstStageMask = pipeline_stages.data();
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd_buffer;
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &render_semaphore;

  if (const VkResult result = vkQueueSubmit(graphics_queue, 1, &submit_info, fence); result != VK_SUCCESS) {
    throw Error("failed to submit draw command buffer").WithCode(result);
  }

  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &render_semaphore;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &swapchain;
  present_info.pImageIndices = &image_idx;

  if (const VkResult result = vkQueuePresentKHR(present_queue, &present_info);
      result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebuffer_resized_) {
    framebuffer_resized_ = false;
    RecreateSwapchain();
  } else if (result != VK_SUCCESS) {
    throw Error("failed to queue present").WithCode(result);
  }
  curr_frame_ = (curr_frame_ + 1) % config_.frame_count;
}

Renderer::~Renderer() {
  vkDeviceWaitIdle(device_.GetLogical());
}

} // namespace vk