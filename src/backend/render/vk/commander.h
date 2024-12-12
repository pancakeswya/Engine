#ifndef BACKEND_RENDER_VK_COMMANDER_H_
#define BACKEND_RENDER_VK_COMMANDER_H_

#include "backend/render/vk/device.h"

#include <vulkan/vulkan.h>

namespace render::vk {

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
  BufferCommander(Device::Dispatchable<VkBuffer>& buffer, VkCommandPool cmd_pool, VkQueue graphics_queue);
  ~BufferCommander() = default;

  void CopyBuffer(const Device::Dispatchable<VkBuffer>& src) const;
private:
  Device::Dispatchable<VkBuffer>& buffer_;
};

class ImageCommander : public SingleTimeCommander {
public:
  ImageCommander(Device::Dispatchable<VkImage>& image, VkCommandPool cmd_pool, VkQueue graphics_queue);
  ~ImageCommander() = default;

  void GenerateMipmaps() const;
  void TransitImageLayout(VkImageLayout old_layout, VkImageLayout new_layout) const;
  void CopyBuffer(const Device::Dispatchable<VkBuffer>& src) const;
private:
  Device::Dispatchable<VkImage>& image_;
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