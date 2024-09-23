#include "backend/render/vk_backend.h"
#include "backend/render/vk_factory.h"

#include <string>

namespace vk {

namespace {

struct ShaderStage {
  VkShaderStageFlagBits bits;
  HandleWrapper<VkShaderModule> module;

  std::string_view name;
};

} // namespace

class BackendImpl {
 public:
  explicit BackendImpl(GLFWwindow* window);
  ~BackendImpl();

  void Render();
 private:
  HandleWrapper<VkInstance> instance_wrapper_;
  HandleWrapper<VkDebugUtilsMessengerEXT> messenger_wrapper_;
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

  HandleWrapper<VkRenderPass> render_pass_wrapper_;
  HandleWrapper<VkPipelineLayout> pipeline_layout_wrapper_;
  HandleWrapper<VkPipeline> pipeline_wrapper_;

  HandleWrapper<VkCommandPool> cmd_pool_wrapper_;
  std::vector<VkCommandBuffer> cmd_buffers_;

  HandleWrapper<VkSemaphore> image_semaphore_wrapper_;
  HandleWrapper<VkSemaphore> render_semaphore_wrapper_;
  HandleWrapper<VkFence> fence_wrapper_;
};

BackendImpl::BackendImpl(GLFWwindow* window) {
  instance_wrapper_ = factory::CreateInstance();
  VkInstance instance = instance_wrapper_.get();

  messenger_wrapper_ = factory::CreateMessenger(instance);

  surface_wrapper_ = factory::CreateSurface(instance, window);
  VkSurfaceKHR surface = surface_wrapper_.get();
  {
    auto[device, indices] = factory::CreatePhysicalDevice(instance, surface);
    physical_device_ = device;
    family_indices_ = indices;
  }
  logical_device_wrapper_ = factory::CreateLogicalDevice(physical_device_, family_indices_);
  VkDevice logical_device = logical_device_wrapper_.get();

  vkGetDeviceQueue(logical_device, family_indices_.graphic, 0, &graphics_queue_);
  vkGetDeviceQueue(logical_device, family_indices_.present, 0, &present_queue_);
  {
    auto[swapchain, details] = factory::CreateSwapchain(window, surface, physical_device_, family_indices_, logical_device);
    swapchain_wrapper_ = std::move(swapchain);
    swapchain_details_ = details;
  }
  VkSwapchainKHR swapchain = swapchain_wrapper_.get();
  swapchain_images_ = factory::CreateSwapchainImages(swapchain, logical_device);

  render_pass_wrapper_ = factory::CreateRenderPass(logical_device, swapchain_details_.format);
  VkRenderPass render_pass = render_pass_wrapper_.get();

  pipeline_layout_wrapper_ = factory::CreatePipelineLayout(logical_device);
  VkPipelineLayout pipeline_layout = pipeline_layout_wrapper_.get();
  {
    const std::array shader_stages = {
        ShaderStage{
          VK_SHADER_STAGE_VERTEX_BIT,
          factory::CreateShaderModule(logical_device, "shaders/vert.spv"),
          "main"
        },
        ShaderStage{
          VK_SHADER_STAGE_FRAGMENT_BIT,
          factory::CreateShaderModule(logical_device,"shaders/frag.spv"),
          "main"
        }
    };
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages_infos;
    shader_stages_infos.reserve(shader_stages.size());
    for(const ShaderStage& shader_stage : shader_stages) {
      VkPipelineShaderStageCreateInfo shader_stage_info = {};
      shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      shader_stage_info.stage = shader_stage.bits;
      shader_stage_info.module = shader_stage.module.get();
      shader_stage_info.pName = shader_stage.name.data();
      shader_stages_infos.push_back(shader_stage_info);
    }
    pipeline_wrapper_ = factory::CreatePipeline(logical_device, pipeline_layout, render_pass, shader_stages_infos);
  }
  image_views_wrapped_ = factory::CreateImageViews(swapchain_images_, logical_device, swapchain_details_.format);
  framebuffers_wrapped_ = factory::CreateFramebuffers(logical_device, image_views_wrapped_, render_pass, swapchain_details_.extent);

  cmd_pool_wrapper_ = factory::CreateCommandPool(logical_device, family_indices_);
  cmd_buffers_ = factory::CreateCommandBuffers(logical_device, cmd_pool_wrapper_.get(), config::kFrameCount);

  image_semaphore_wrapper_ = factory::CreateSemaphore(logical_device);
  render_semaphore_wrapper_ = factory::CreateSemaphore(logical_device);

  fence_wrapper_ = factory::CreateFence(logical_device);
}

void BackendImpl::Render() {
  uint32_t image_idx;

  VkDevice logical_device = logical_device_wrapper_.get();
  VkFence fence = fence_wrapper_.get();
  VkSwapchainKHR swapchain = swapchain_wrapper_.get();
  VkSemaphore image_semaphore = image_semaphore_wrapper_.get();
  VkSemaphore render_semaphore = render_semaphore_wrapper_.get();
  VkRenderPass render_pass = render_pass_wrapper_.get();
  VkCommandBuffer cmd_buffer = cmd_buffers_[0];
  VkPipeline pipeline = pipeline_wrapper_.get();

  if (const VkResult result = vkWaitForFences(logical_device, 1, &fence, VK_TRUE, UINT64_MAX); result != VK_SUCCESS) {
    throw Error("failed to wait for fences").WithCode(result);
  }
  if (const VkResult result = vkResetFences(logical_device, 1, &fence); result != VK_SUCCESS) {
    throw Error("failed to reset fences").WithCode(result);
  }
  if (const VkResult result = vkAcquireNextImageKHR(logical_device, swapchain, UINT64_MAX, image_semaphore, VK_NULL_HANDLE, &image_idx); result != VK_SUCCESS) {
    throw Error("failed to acquire next image").WithCode(result);
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

  vkCmdDraw(cmd_buffer, 3, 1, 0, 0);
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

  if (const VkResult result = vkQueuePresentKHR(present_queue_, &present_info); result != VK_SUCCESS) {
    throw Error("failed to queue present").WithCode(result);
  }
}

BackendImpl::~BackendImpl() {
  vkDeviceWaitIdle(logical_device_wrapper_.get());
}

Backend::Backend(GLFWwindow* window)
  : window_(window), impl_(new BackendImpl(window))  {}

Backend::~Backend() { delete impl_; }

void Backend::Render() {
  impl_->Render();
}

} // vk