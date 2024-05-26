#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <vulkan/vulkan.h>

namespace vk {

class Pipeline {
public:
  static constexpr VkPipelineStageFlags kStageFlags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

  class Layout {
  public:
    explicit Layout(
      VkDevice logical_device
    );
    ~Layout();

    VkPipelineLayout get() noexcept;
  private:
    VkDevice logical_device_;
    VkPipelineLayout layout_;
  };

  Pipeline(
    VkDevice logical_device,
    VkPipelineLayout layout,
    VkRenderPass pass
  );
  ~Pipeline();
  VkPipeline get() noexcept;

private:
  VkDevice logical_device_;
  VkPipeline pipeline_;
};

inline VkPipelineLayout Pipeline::Layout::get() noexcept {
  return layout_;
}

inline VkPipeline Pipeline::get() noexcept {
  return pipeline_;
}

} // namespace vk::graphics

#endif // GRAPHICS_H_