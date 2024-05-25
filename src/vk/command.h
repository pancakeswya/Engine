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

  VkCommandPool Get() noexcept;
private:
  VkDevice logical_device_;
  VkCommandPool pool_;
};

inline VkCommandPool Pool::Get() noexcept {
  return pool_;
}

class Buffer {
public:
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
    friend Buffer;

    Record(VkCommandBuffer buffer);
    VkCommandBuffer buffer_;
  };

  Buffer(VkDevice logical_device, VkCommandPool pool);
  ~Buffer() = default;

  void Reset() noexcept;

  VkCommandBuffer Get() noexcept;

  Record BeginRecord();

private:
  VkCommandBuffer buffer_;
};

inline VkCommandBuffer Buffer::Get() noexcept {
  return buffer_;
}

inline void Buffer::Reset() noexcept {
  vkResetCommandBuffer(buffer_,0);
}

inline void Buffer::Record::BeginRenderPass(VkRenderPassBeginInfo* pass_info) noexcept {
  vkCmdBeginRenderPass(buffer_, pass_info, VK_SUBPASS_CONTENTS_INLINE);
}

inline void Buffer::Record::BindPipeline(VkPipeline pipeline) noexcept {
  vkCmdBindPipeline(buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

inline void Buffer::Record::SetViewport(VkViewport* viewport) noexcept {
  vkCmdSetViewport(buffer_, 0, 1, viewport);
}

inline void Buffer::Record::SetScissor(VkRect2D* scissor) noexcept {
  vkCmdSetScissor(buffer_, 0, 1, scissor);
}

inline void Buffer::Record::Draw() noexcept {
  vkCmdDraw(buffer_, 3, 1, 0, 0);
}

inline void Buffer::Record::EndRenderPass() noexcept {
  vkCmdEndRenderPass(buffer_);
}

} // namespace command

#endif // VK_COMMAND_H_