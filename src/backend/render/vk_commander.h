#ifndef BACKEND_RENDER_VK_COMMANDER_H_
#define BACKEND_RENDER_VK_COMMANDER_H_

#include <vulkan/vulkan.h>

namespace vk {

class Buffer;
class Image;

class SingleTimeCommander {
public:
  SingleTimeCommander(VkDevice logical_device, VkCommandPool cmd_pool, VkQueue graphics_queue);
  ~SingleTimeCommander();

  void Begin();
  void End();
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

  void CopyBuffer(const Buffer& src);
private:
  Buffer& buffer_;
};

class ImageCommander : public SingleTimeCommander {
public:
  ImageCommander(Image& image, VkCommandPool cmd_pool, VkQueue graphics_queue);
  ~ImageCommander() = default;

  void GenerateMipmaps();
  void TransitImageLayout(VkImageLayout old_layout, VkImageLayout new_layout);
  void CopyBuffer(const Buffer& src);
private:
  Image& image_;
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