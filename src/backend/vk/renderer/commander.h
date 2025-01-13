#ifndef BACKEND_VK_RENDERER_COMMANDER_H_
#define BACKEND_VK_RENDERER_COMMANDER_H_

#include "backend/vk/renderer/device.h"

#include <vulkan/vulkan.h>

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
  BufferCommander(Device::Dispatchable<VkBuffer>& buffer, VkCommandPool cmd_pool, VkQueue graphics_queue);
  ~BufferCommander() = default;

  void CopyBuffer(const Device::Dispatchable<VkBuffer>& src) const;
private:
  Device::Dispatchable<VkBuffer>& buffer_;
};

class ImageCommander : public Commander {
public:
  ImageCommander(Device::Dispatchable<VkImage>& image, VkCommandPool cmd_pool, VkQueue graphics_queue);
  ~ImageCommander() = default;

  void GenerateMipmaps() const;
  void TransitImageLayout(VkImageLayout old_layout, VkImageLayout new_layout) const;
  void CopyBuffer(const Device::Dispatchable<VkBuffer>& src) const;
private:
  Device::Dispatchable<VkImage>& image_;
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