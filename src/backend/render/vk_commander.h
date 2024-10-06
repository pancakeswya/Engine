#ifndef BACKEND_RENDER_VK_COMMANDER_H_
#define BACKEND_RENDER_VK_COMMANDER_H_

#include "backend/render/vk_types.h"
#include "backend/render/vk_factory.h"

#include <vulkan/vulkan.h>

namespace vk {

class CommanderSingle {
public:
  DECL_UNIQUE_OBJECT(CommanderSingle);

  CommanderSingle(VkDevice logical_device, VkCommandPool cmd_pool) {
    std::vector<VkCommandBuffer> cmd_buffers = factory::CreateCommandBuffers(logical_device, cmd_pool, 1);
    cmd_buffer_wrapper_ = HandleWrapper<VkCommandBuffer>(
      cmd_buffers[0],
      [logical_device, cmd_pool](VkCommandBuffer cmd_buffer) {
        vkFreeCommandBuffers(logical_device, cmd_pool, 1, &cmd_buffer);
      }
    );
  }

  ~CommanderSingle() = default;

  void Begin() {
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (const VkResult result = vkBeginCommandBuffer(cmd_buffer_wrapper_.get(), &begin_info); result != VK_SUCCESS) {
      throw Error("failed to begin command buffer").WithCode(result);
    }
  }

  void QueueSubmit(VkQueue queue) {
    VkCommandBuffer cmd_buffer = cmd_buffer_wrapper_.get();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd_buffer;

    if (const VkResult result = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE); result != VK_SUCCESS) {
      throw Error("failed to submin the graphics queue");
    }

  }

  void QueueWaitIdle(VkQueue queue) {
    if (const VkResult result = vkQueueWaitIdle(queue); result != VK_SUCCESS) {
      throw Error("failed to wait idle graphics queue").WithCode(result);
    }
  }

  void End() {
    if (const VkResult result = vkEndCommandBuffer(cmd_buffer_wrapper_.get()); result != VK_SUCCESS) {
      throw Error("failed to end cmd buffer").WithCode(result);
    }
  }
private:
  HandleWrapper<VkCommandBuffer> cmd_buffer_wrapper_;
};

template<typename CommanderType>
class CommandGuard {
public:
  CommandGuard(CommanderType& commander) : commander_(commander) {
    commander_.Begin();
  }
  ~CommandGuard() {
    commander_.End();
  }
private:
  CommanderType& commander_;
};

} // namespace vk

#endif // BACKEND_RENDER_VK_COMMANDER_H_