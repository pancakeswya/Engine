#include "backend/vk/renderer/renderer.h"

#include <array>
#include <cstring>

#include "backend/vk/renderer/device_selector.h"
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
  ObjectLoader::Init();

  window.SetWindowUserPointer(this);
  window.SetWindowResizedCallback([](void* user_ptr, [[maybe_unused]] int width, [[maybe_unused]] int height) {
    auto render = static_cast<Renderer*>(user_ptr);
    render->framebuffer_resized_ = true;
  });
#ifdef DEBUG
  messenger_ = instance_.CreateMessenger();
#endif

  surface_ = instance_.CreateSurface(window);

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

  std::optional<Device> device = DeviceSelector(devices).Select(requirements);
  if (!device) {
    throw Error("failed to find suitable device");
  }
  device_ = std::move(*device);

  std::tie(swapchain_, depth_image_) = CreateSwapchainAndDepthImage();
  render_pass_ = device_.CreateRenderPass(swapchain_.GetFormat(),  depth_image_.GetFormat());
  std::tie(swapchain_framebuffers_, sync_objects_) = CreateSwapchainImagesAndSyncObjects();

  cmd_pool_ = device_.CreateCommandPool(device_.GetGraphicsQueue().family_index);
  cmd_buffers_ = device_.CreateCommandBuffers(cmd_pool_.GetHandle(), config_.frame_count);
}

Renderer::~Renderer() {
  vkDeviceWaitIdle(device_.GetHandle());
}

void Renderer::RenderFrame() {
  uint32_t image_idx;

  VkFence fence = sync_objects_[curr_frame_].fence.GetHandle();
  VkSemaphore image_semaphore = sync_objects_[curr_frame_].image_semaphore.GetHandle();
  VkSemaphore render_semaphore = sync_objects_[curr_frame_].render_semaphore.GetHandle();

  VkCommandBuffer cmd_buffer = cmd_buffers_[curr_frame_];
  VkSwapchainKHR swapchain = swapchain_.GetHandle();

  if (const VkResult result = vkWaitForFences(device_.GetHandle(), 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()); result != VK_SUCCESS) {
    throw Error("failed to wait for fences").WithCode(result);
  }
  if (const VkResult result = vkAcquireNextImageKHR(device_.GetHandle(), swapchain_.GetHandle(), std::numeric_limits<uint64_t>::max(), image_semaphore, VK_NULL_HANDLE, &image_idx); result != VK_SUCCESS) {
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
      RecreateSwapchain();
      return;
    }
    throw Error("failed to acquire next image").WithCode(result);
  }
  UpdateUniforms();
  if (const VkResult result = vkResetFences(device_.GetHandle(), 1, &fence); result != VK_SUCCESS) {
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

  if (const VkResult result = vkQueueSubmit(device_.GetGraphicsQueue().handle, 1, &submit_info, fence); result != VK_SUCCESS) {
    throw Error("failed to submit draw command buffer").WithCode(result);
  }

  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &render_semaphore;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &swapchain;
  present_info.pImageIndices = &image_idx;

  if (const VkResult result = vkQueuePresentKHR(device_.GetGraphicsQueue().handle, &present_info);
      result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebuffer_resized_) {
    framebuffer_resized_ = false;
    RecreateSwapchain();
  } else if (result != VK_SUCCESS) {
    throw Error("failed to queue present").WithCode(result);
  }
  curr_frame_ = (curr_frame_ + 1) % config_.frame_count;
}

void Renderer::LoadModel(const std::string& path) {
  object_ = ObjectLoader(device_, config_.image_settings, cmd_pool_.GetHandle()).Load(path, config_.frame_count);

  const std::vector descriptor_set_layouts = { object_.uniform_descriptor.layout.GetHandle(), object_.sampler_descriptor.layout.GetHandle() };

  pipeline_layout_ = device_.CreatePipelineLayout(descriptor_set_layouts);

  const std::vector<ShaderInfo> shader_infos = Shader::GetInfos();

  std::vector<Shader> shaders;
  shaders.reserve(shader_infos.size());
  for(const auto& [description, spirv] : shader_infos) {
    Shader shader = {};
    shader.module = device_.CreateShaderModule(spirv);
    shader.description = description;

    shaders.emplace_back(std::move(shader));
  }
  pipeline_ = device_.CreatePipeline(pipeline_layout_.GetHandle(), render_pass_.GetHandle(), Vertex::GetAttributeDescriptions(), Vertex::GetBindingDescriptions(), shaders);

  uniforms_buff_.reserve(object_.uniform_descriptor.sets.size());
  for(const UniformDescriptorSet& descriptor_set : object_.uniform_descriptor.sets) {
    auto uniforms = static_cast<Uniforms*>(descriptor_set.buffer.GetMemory().Map());
    uniforms_buff_.emplace_back(uniforms);
  }
}

void Renderer::RecreateSwapchain() {
  window_.WaitUntilResized();

  if (const VkResult result = vkDeviceWaitIdle(device_.GetHandle()); result != VK_SUCCESS) {
    throw Error("failed to idle device");
  }
  swapchain_ = Swapchain();

  std::tie(swapchain_, depth_image_) = CreateSwapchainAndDepthImage();
  std::tie(swapchain_framebuffers_, sync_objects_) = CreateSwapchainImagesAndSyncObjects();
}

std::pair<Swapchain, Image> Renderer::CreateSwapchainAndDepthImage() const {
  VkExtent2D swapchain_extent = {};
  swapchain_extent.width = window_.GetWidth();
  swapchain_extent.height = window_.GetHeight();

  Swapchain swapchain = device_.CreateSwapchain(swapchain_extent, surface_.GetHandle());

  VkFormat depth_format = device_.GetPhysicalDevice().FindSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
  );
  Image depth_image = device_.CreateImage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, swapchain.GetExtent(), depth_format, VK_IMAGE_TILING_OPTIMAL);
  device_.AllocateImage(depth_image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  device_.BindImage(depth_image);
  device_.MakeImageView(depth_image, VK_IMAGE_ASPECT_DEPTH_BIT);

  return { std::move(swapchain), std::move(depth_image) };
}

std::pair<std::vector<SwapchainFramebuffer>, std::vector<SyncObject>> Renderer::CreateSwapchainImagesAndSyncObjects() const {
  const std::vector<VkImage> images = swapchain_.GetImages();
  std::vector<SwapchainFramebuffer> swapchain_framebuffers;
  swapchain_framebuffers.reserve(images.size());

  for(VkImage image : images) {
    SwapchainFramebuffer swapchain_framebuffer = {};
    swapchain_framebuffer.view = device_.CreateImageView(image, VK_IMAGE_ASPECT_COLOR_BIT, swapchain_.GetFormat());
    swapchain_framebuffer.framebuffer = device_.CreateFramebuffer({swapchain_framebuffer.view.GetHandle(), depth_image_.GetView()}, render_pass_.GetHandle(), swapchain_.GetExtent());

    swapchain_framebuffers.emplace_back(std::move(swapchain_framebuffer));
  }
  const size_t count = config_.frame_count;

  std::vector<SyncObject> sync_objects;
  sync_objects.reserve(count);

  for(size_t i = 0; i < count; ++i) {
    SyncObject sync_object = {};
    sync_object.image_semaphore = device_.CreateSemaphore();
    sync_object.render_semaphore = device_.CreateSemaphore();
    sync_object.fence = device_.CreateFence();

    sync_objects.emplace_back(std::move(sync_object));
  }

  return { std::move(swapchain_framebuffers), std::move(sync_objects) };
}

inline void Renderer::UpdateUniforms() const {
  const engine::Uniforms& uniforms = model_.GetUniforms();
  std::memcpy(uniforms_buff_[curr_frame_], &uniforms, sizeof(Uniforms));
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
  render_pass_begin_info.framebuffer = swapchain_framebuffers_[image_idx].framebuffer.GetHandle();
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
    vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_.GetHandle(), 0, 1, &object_.uniform_descriptor.sets[curr_frame_].handle, 0, nullptr);
    vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_.GetHandle(), 1, 1, &object_.sampler_descriptor.sets[index].handle, 0, nullptr);

    vkCmdDrawIndexed(cmd_buffer, static_cast<uint32_t>(offset - prev_offset), 1, 0, 0, 0);

    prev_offset = offset;
  }
  vkCmdEndRenderPass(cmd_buffer);
  if (const VkResult result = vkEndCommandBuffer(cmd_buffer); result != VK_SUCCESS) {
    throw Error("failed to record command buffer").WithCode(result);
  }
}

} // namespace vk