#ifndef BACKEND_RENDER_VK_COMMANDER_H_
#define BACKEND_RENDER_VK_COMMANDER_H_

#include <vulkan/vulkan.h>

#include "vk_types.h"

namespace vk {

class Buffer;
class Image;
class Object;

class SingleTimeCommander {
public:
  SingleTimeCommander(VkDevice logical_device, VkCommandPool cmd_pool, VkQueue graphics_queue);
  ~SingleTimeCommander();

  void Begin() const;
  void End() const;
protected:
  VkDevice logical_device_;
  VkCommandPool cmd_pool_;
  VkCommandBuffer cmd_buffer_;
  VkQueue graphics_queue_;
};

class BufferCommander : public SingleTimeCommander {
public:
  BufferCommander(Buffer& buffer, VkCommandPool cmd_pool, VkQueue graphics_queue);
  ~BufferCommander() = default;

  void CopyBuffer(const Buffer& src) const;
private:
  Buffer& buffer_;
};

class ImageCommander : public SingleTimeCommander {
public:
  ImageCommander(Image& image, VkCommandPool cmd_pool, VkQueue graphics_queue);
  ~ImageCommander() = default;

  void GenerateMipmaps() const;
  void TransitImageLayout(VkImageLayout old_layout, VkImageLayout new_layout) const;
  void CopyBuffer(const Buffer& src) const;
private:
  Image& image_;
};

class RenderCommander {
public:
  DECL_UNIQUE_OBJECT(RenderCommander);

  RenderCommander(VkDevice logical_device, VkCommandPool cmd_pool);
  ~RenderCommander() = default;

  void Next() noexcept;

  void Begin() const;
  void End() const;

  void BeginRender(VkRenderPass render_pass, VkFramebuffer framebuffer, VkPipeline pipeline, VkExtent2D extent) const;
  void EndRender() const;

  void Submit(VkQueue graphics_queue, VkFence fence, VkSemaphore wait_semaphore, VkSemaphore signal_semaphore) const;

  void DrawObject(Object& object, VkPipelineLayout pipeline_layout, VkDescriptorSet descriptor_set) const;
private:
  size_t curr_buffer_;
  std::vector<VkCommandBuffer> cmd_buffers_;
};

template<typename CommanderType>
class CommandGuard {
public:
  explicit CommandGuard(CommanderType& commander) : commander_(commander) {
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