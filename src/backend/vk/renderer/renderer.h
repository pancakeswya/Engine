#ifndef BACKEND_VK_RENDERER_RENDERER_H_
#define BACKEND_VK_RENDERER_RENDERER_H_

#include <string>
#include <vector>
#include <utility>

#ifdef __APPLE__
#define VK_ENABLE_BETA_EXTENSIONS
#endif

#include <vulkan/vulkan.h>

#include "engine/render/model.h"
#include "engine/render/renderer.h"

#include "backend/vk/renderer/window.h"
#include "backend/vk/renderer/config.h"
#include "backend/vk/renderer/device.h"
#include "backend/vk/renderer/instance.h"
#include "backend/vk/renderer/swapchain.h"
#include "backend/vk/renderer/object.h"

namespace vk {

struct SwapchainFramebuffer {
  DeviceDispatchable<VkFramebuffer> framebuffer;
  DeviceDispatchable<VkImageView> view;
};

struct SyncObject {
  DeviceDispatchable<VkSemaphore> image_semaphore;
  DeviceDispatchable<VkSemaphore> render_semaphore;
  DeviceDispatchable<VkFence> fence;
};

class Renderer final : public engine::Renderer {
public:
  explicit Renderer(Config config, Window& window);
  ~Renderer() override;

  void RenderFrame() override;
  void LoadModel(const std::string& path) override;
  engine::Model& GetModel() noexcept override;
private:
  void RecreateSwapchain();
  std::pair<Swapchain, Image> CreateSwapchainAndDepthImage() const;
  std::pair<std::vector<SwapchainFramebuffer>, std::vector<SyncObject>> CreateSwapchainImagesAndSyncObjects() const;

  void UpdateUniforms() const;
  void RecordCommandBuffer(VkCommandBuffer cmd_buffer, size_t image_idx);

  Config config_;

  Window& window_;

  bool framebuffer_resized_;
  mutable size_t curr_frame_;

  Instance instance_;
#ifdef DEBUG
  InstanceDispatchable<VkDebugUtilsMessengerEXT> messenger_;
#endif // DEBUG
  InstanceDispatchable<VkSurfaceKHR> surface_;

  Device device_;

  Swapchain swapchain_;
  Image depth_image_;
  DeviceDispatchable<VkRenderPass> render_pass_;
  std::vector<SwapchainFramebuffer> swapchain_framebuffers_;
  std::vector<SyncObject> sync_objects_;

  DeviceDispatchable<VkCommandPool> cmd_pool_;
  std::vector<VkCommandBuffer> cmd_buffers_;

  DeviceDispatchable<VkPipelineLayout> pipeline_layout_;
  DeviceDispatchable<VkPipeline> pipeline_;

  Object object_;
  std::vector<Uniforms*> uniforms_buff_;
  engine::Model model_;
};

inline engine::Model& Renderer::GetModel() noexcept {
  return model_;
}

} // namespace vk

#endif // BACKEND_VK_RENDERER_RENDERER_H_