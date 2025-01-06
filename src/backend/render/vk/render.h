#ifndef BACKEND_RENDER_VK_RENDER_H_
#define BACKEND_RENDER_VK_RENDER_H_

#include <string>
#include <vector>

#include "backend/render/model.h"
#include "backend/render/vk/config.h"
#include "backend/render/vk/device.h"
#include "backend/render/vk/instance.h"
#include "backend/render/vk/object.h"

namespace window {

class IWindow;

} // namespace window

namespace render::vk {

class Renderer final {
public:
  explicit Renderer(Config config, window::IWindow& window);
  ~Renderer();

  void RenderFrame();
  void LoadModel(const std::string& path);
  [[nodiscard]] Model& GetModel() noexcept;
private:
  void RecreateSwapchain();
  void UpdateUniforms() const;
  void RecordCommandBuffer(VkCommandBuffer cmd_buffer, size_t image_idx);

  Config config_;

  window::IWindow& window_;

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

  Model model_;
};

inline Model& Renderer::GetModel() noexcept {
  return model_;
}

} // namespace vk::backend

#endif // BACKEND_RENDER_VK_RENDER_H_