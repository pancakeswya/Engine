#ifndef VK_COMMAND_H_
#define VK_COMMAND_H_

#include <vulkan/vulkan.h>

namespace vk::command {

class Pool {
public:
  Pool(
    VkDevice logical_device,
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface
  );
  ~Pool();

  VkCommandPool get() noexcept;
private:
  VkDevice logical_device_;
  VkCommandPool pool_;
};

inline VkCommandPool Pool::get() noexcept {
  return pool_;
}

class Record {
public:
  Record(VkCommandBuffer buffer);
  void Begin();
  void BeginRenderPass(VkRenderPassBeginInfo* pass_info) noexcept;
  void BindPipeline(VkPipeline pipeline) noexcept;
  void SetViewport(VkViewport* viewport) noexcept;
  void SetScissor(VkRect2D* scissor) noexcept;
  void Draw() noexcept;
  void EndRenderPass() noexcept;
  void End();
private:
  VkCommandBuffer buffer_;
};

class Buffers {
public:
  Buffers(VkDevice logical_device, VkCommandPool pool, uint32_t count);
  ~Buffers();

  VkCommandBuffer operator[](uint32_t idx) noexcept;
private:
  VkCommandBuffer* buffers_;
};

inline VkCommandBuffer Buffers::operator[](const uint32_t idx) noexcept {
  return buffers_[idx];
}

inline void Record::BeginRenderPass(VkRenderPassBeginInfo* pass_info) noexcept {
  vkCmdBeginRenderPass(buffer_, pass_info, VK_SUBPASS_CONTENTS_INLINE);
}

inline void Record::BindPipeline(VkPipeline pipeline) noexcept {
  vkCmdBindPipeline(buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

inline void Record::SetViewport(VkViewport* viewport) noexcept {
  vkCmdSetViewport(buffer_, 0, 1, viewport);
}

inline void Record::SetScissor(VkRect2D* scissor) noexcept {
  vkCmdSetScissor(buffer_, 0, 1, scissor);
}

inline void Record::Draw() noexcept {
  vkCmdDraw(buffer_, 3, 1, 0, 0);
}

inline void Record::EndRenderPass() noexcept {
  vkCmdEndRenderPass(buffer_);
}

} // namespace command

#endif // VK_COMMAND_H_