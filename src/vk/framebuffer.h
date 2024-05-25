#ifndef VK_FRAMEBUFFER_H_
#define VK_FRAMEBUFFER_H_
#include <vulkan/vulkan_core.h>

namespace vk {

class Framebuffer {
public:
  Framebuffer(
    VkDevice logical_device,
    VkRenderPass render_pass,
    VkImageView view,
    VkExtent2D extent
  );
  ~Framebuffer();
  VkFramebuffer Get() noexcept;
private:
  VkDevice logical_device_;
  VkFramebuffer framebuffer_;
};

inline VkFramebuffer Framebuffer::Get() noexcept {
  return framebuffer_;
}

} // namespace vk

#endif // VK_FRAMEBUFFER_H_