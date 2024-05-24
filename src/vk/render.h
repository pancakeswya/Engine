#ifndef VK_RENDER_PASS_H_
#define VK_RENDER_PASS_H_

#include <vulkan/vulkan.h>

namespace vk::render {

class Pass {
public:
  Pass(
    VkDevice logical_device,
    VkFormat swap_chain_format
  );
  ~Pass();

  VkRenderPass Get() noexcept;
private:
  VkDevice logical_device_;
  VkRenderPass pass_;
};

inline VkRenderPass Pass::Get() noexcept {
  return pass_;
}


} // namespace vk

#endif // VK_RENDER_PASS_H_