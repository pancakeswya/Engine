#ifndef VK_RENDER_PASS_H_
#define VK_RENDER_PASS_H_

#include <vulkan/vulkan.h>

namespace vk {

class RenderPass {
public:
  RenderPass(
    VkDevice logical_device,
    VkFormat swap_chain_format
  );
  ~RenderPass();
  VkRenderPassBeginInfo BeginInfo(
    VkFramebuffer buffer,
    VkExtent2D extent,
    VkClearValue* clear_color
  ) noexcept;

  VkRenderPass get() noexcept;
private:
  VkDevice logical_device_;
  VkRenderPass pass_;
};

inline VkRenderPass RenderPass::get() noexcept {
  return pass_;
}


} // namespace vk

#endif // VK_RENDER_PASS_H_