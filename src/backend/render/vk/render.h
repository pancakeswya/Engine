#ifndef BACKEND_RENDER_VK_RENDER_H_
#define BACKEND_RENDER_VK_RENDER_H_

#include "backend/render/model.h"
#include "backend/render/vk/device.h"
#include "backend/render/vk/instance.h"
#include "backend/render/vk/object.h"

#include <string>
#include <vector>

namespace window {

class IWindow;

} // namespace window

namespace render::vk {

class Renderer final {
public:
  explicit Renderer(window::IWindow& window);
  ~Renderer();

  void RenderFrame();
  void LoadModel(const std::string& path);
  [[nodiscard]] const render::Model& GetModel() const noexcept;
private:
  void RecreateSwapchain();
  void RecordCommandBuffer(VkCommandBuffer cmd_buffer, size_t image_idx);

  bool framebuffer_resized_;
  size_t curr_frame_;

  window::IWindow& window_;

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
  ObjectLoader object_loader_;

  Device::Dispatchable<VkPipelineLayout> pipeline_layout_;
  Device::Dispatchable<VkPipeline> pipeline_;

  std::vector<Model> models_;
};

inline const render::Model& Renderer::GetModel() const noexcept {
  return models_[curr_frame_];
}

} // namespace vk::backend

#endif // BACKEND_RENDER_VK_RENDER_H_