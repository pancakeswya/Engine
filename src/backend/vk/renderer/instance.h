#ifndef BACKEND_VK_RENDERER_INSTANCE_H_
#define BACKEND_VK_RENDERER_INSTANCE_H_

#include "backend/vk/renderer/dispatchable.h"
#include "backend/vk/window/window.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace vk {

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
  [[nodiscard]] Dispatchable<VkSurfaceKHR> CreateSurface(const SurfaceFactory& surface_factory) const;

  explicit Instance(const VkApplicationInfo& app_info, const std::vector<const char*>& extensions, const VkAllocationCallbacks* allocator = nullptr);
  ~Instance();

  [[nodiscard]] std::vector<VkPhysicalDevice> EnumeratePhysicalDevices() const;
private:
  VkInstance handle_;
  const VkAllocationCallbacks* allocator_;
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_INSTANCE_H_