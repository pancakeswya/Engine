#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <vulkan/vulkan.h>

namespace vk::graphics {

class Pipeline {
public:
  class Layout {
  public:
    explicit Layout(
      VkDevice logical_device
    );
    ~Layout();

    VkPipelineLayout Get() noexcept;
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
  VkPipeline Get() noexcept;

private:
  VkDevice logical_device_;
  VkPipeline pipeline_;
};

inline VkPipelineLayout Pipeline::Layout::Get() noexcept {
  return layout_;
}

inline VkPipeline Pipeline::Get() noexcept {
  return pipeline_;
}

} // namespace vk::graphics

#endif // GRAPHICS_H_