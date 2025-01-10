#ifndef BACKEND_RENDER_VK_INSTANCE_H_
#define BACKEND_RENDER_VK_INSTANCE_H_

#include "backend/render/vk/dispatchable.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace window::vk {

class SurfaceFactory;

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

  static std::vector<const char*> GetLayers();

#ifdef DEBUG
  Dispatchable<VkDebugUtilsMessengerEXT> CreateMessenger() const;
#endif
  [[nodiscard]] Dispatchable<VkSurfaceKHR> CreateSurface(const window::vk::SurfaceFactory& surface_factory) const;

  explicit Instance(const VkApplicationInfo& app_info, const std::vector<const char*>& extensions, const VkAllocationCallbacks* allocator = nullptr);
  ~Instance();

  [[nodiscard]] std::vector<VkPhysicalDevice> EnumeratePhysicalDevices() const;
private:
  VkInstance handle_;
  const VkAllocationCallbacks* allocator_;
};

} // namespace vk

#endif // BACKEND_RENDER_VK_INSTANCE_H_