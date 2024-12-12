#ifndef BACKEND_RENDER_VK_INSTANCE_H_
#define BACKEND_RENDER_VK_INSTANCE_H_

#include "backend/render/vk/dispatchable.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace window {

class ISurfaceFactory;

} // namespace window

namespace render::vk {

class Instance {
public:
  using HandleType = VkInstance;

  template<typename Tp>
  class Dispatchable : public vk::Dispatchable<Tp, Instance> {
  public:
    using Base = vk::Dispatchable<Tp, Instance>;
    using Base::Base;
  };

#ifdef DEBUG
  Dispatchable<VkDebugUtilsMessengerEXT> CreateMessenger() const;
#endif
  [[nodiscard]] Dispatchable<VkSurfaceKHR> CreateSurface(const window::ISurfaceFactory& surface_factory) const;

  explicit Instance(const std::vector<const char*>& extensions, const VkAllocationCallbacks* allocator = nullptr);
  ~Instance();

  [[nodiscard]] std::vector<VkPhysicalDevice> EnumeratePhysicalDevices() const;
private:
  VkInstance handle_;
  const VkAllocationCallbacks* allocator_;
};

} // namespace vk

#endif // BACKEND_RENDER_VK_INSTANCE_H_