#ifndef BACKEND_VK_RENDERER_RENDERER_H_
#define BACKEND_VK_RENDERER_RENDERER_H_

#include <string>
#include <vector>

#include "engine/render/model.h"
#include "engine/render/renderer.h"

#include "backend/vk/renderer/renderer.h"
#include "backend/vk/renderer/config.h"
#include "backend/vk/renderer/device.h"
#include "backend/vk/renderer/instance.h"
#include "backend/vk/renderer/object.h"
#include "backend/vk/renderer/window.h"

namespace vk {

class Renderer final : public engine::Renderer {
public:
  explicit Renderer(Config config, Window& window);
  ~Renderer() override;

  void RenderFrame() override;
  void LoadModel(const std::string& path) override;
  engine::Model& GetModel() noexcept override;
private:
  void RecreateSwapchain();
  void UpdateUniforms();
  void RecordCommandBuffer(VkCommandBuffer cmd_buffer, size_t image_idx);

  Config config_;

  Window& window_;

  bool framebuffer_resized_;
  mutable size_t curr_frame_;

  Instance instance_;
#ifdef DEBUG
  Instance::Dispatchable<VkDebugUtilsMessengerEXT> messenger_;
#endif // DEBUG
  Instance::Dispatchable<VkSurfaceKHR> surface_;

  Device device_;

  Device::Dispatchable<VkSwapchainKHR> swapchain_;
  std::vector<Device::Dispatchable<VkFramebuffer>> framebuffers_;

  Device::Dispatchable<VkRenderPass> render_pass_;

  Device::Dispatchable<VkCommandPool> cmd_pool_;
  std::vector<VkCommandBuffer> cmd_buffers_;

  std::vector<Device::Dispatchable<VkSemaphore>> image_semaphores_;
  std::vector<Device::Dispatchable<VkSemaphore>> render_semaphores_;
  std::vector<Device::Dispatchable<VkFence>> fences_;

  Object object_;

  Device::Dispatchable<VkPipelineLayout> pipeline_layout_;
  Device::Dispatchable<VkPipeline> pipeline_;

  std::vector<Uniforms*> uniforms_buff_;

  engine::Model model_;
};

inline engine::Model& Renderer::GetModel() noexcept {
  return model_;
}

} // namespace vk

#endif // BACKEND_VK_RENDERER_RENDERER_H_