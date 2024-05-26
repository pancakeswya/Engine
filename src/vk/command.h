#ifndef VK_COMMAND_H_
#define VK_COMMAND_H_

#include "vulkan/vulkan.h"

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
  void BeginRenderPass(VkRenderPassBeginInfo* pass_info) noexcept;
  void BindPipeline(VkPipeline pipeline) noexcept;
  void SetViewport(VkViewport* viewport) noexcept;
  void SetScissor(VkRect2D* scissor) noexcept;
  void Draw() noexcept;
  void EndRenderPass() noexcept;

  void End();
private:
  friend class Buffer;

  Record(VkCommandBuffer buffer);
  VkCommandBuffer buffer_;
};

class Buffer {
public:
  Buffer(VkDevice logical_device, VkCommandPool pool);
  ~Buffer() = default;

  void Reset() noexcept;

  VkCommandBuffer get() noexcept;

  Record BeginRecord();

private:
  VkCommandBuffer buffer_;
};

inline VkCommandBuffer Buffer::get() noexcept {
  return buffer_;
}

inline void Buffer::Reset() noexcept {
  vkResetCommandBuffer(buffer_,0);
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