#ifndef BACKEND_VK_RENDERER_COMMANDER_H_
#define BACKEND_VK_RENDERER_COMMANDER_H_

#include <vulkan/vulkan.h>

#include "backend/vk/renderer/buffer.h"
#include "backend/vk/renderer/image.h"

namespace vk {

class Commander {
public:
  Commander(VkDevice logical_device, VkCommandPool cmd_pool, VkQueue graphics_queue);
  ~Commander();

  void Begin() const;
  void End() const;
protected:
  VkDevice logical_device_;
  VkCommandPool cmd_pool_;
  VkCommandBuffer cmd_buffer_;
  VkQueue graphics_queue_;
};

class BufferCommander : public Commander {
public:
  BufferCommander(Buffer& buffer, VkCommandPool cmd_pool, VkQueue graphics_queue);
  ~BufferCommander() = default;

  void CopyBuffer(const Buffer& src) const;
private:
  Buffer& buffer_;
};

class ImageCommander : public Commander {
public:
  ImageCommander(Image& image, VkCommandPool cmd_pool, VkQueue graphics_queue);
  ~ImageCommander() = default;

  void GenerateMipmaps() const;
  void TransitImageLayout(VkImageLayout old_layout, VkImageLayout new_layout) const;
  void CopyBuffer(const Buffer& src) const;
private:
  Image& image_;
};

class CommanderGuard {
public:
  explicit CommanderGuard(Commander& commander) : commander_(commander) {
    commander_.Begin();
  }

  ~CommanderGuard() {
    commander_.End();
  }
private:
  Commander& commander_;
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_COMMANDER_H_