#ifndef BACKEND_RENDER_VK_INSTANCE_H_
#define BACKEND_RENDER_VK_INSTANCE_H_

#include "backend/render/vk/dispatchable.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

#ifdef DEBUG
  Dispatchable<VkDebugUtilsMessengerEXT> CreateMessenger() const;
#endif
  Dispatchable<VkSurfaceKHR> CreateSurface(GLFWwindow* window) const;

  explicit Instance(const VkAllocationCallbacks* allocator = nullptr);
  ~Instance();

  [[nodiscard]] std::vector<VkPhysicalDevice> EnumeratePhysicalDevices() const;
private:
  VkInstance handle_;
  const VkAllocationCallbacks* allocator_;
};

} // namespace vk

#endif // BACKEND_RENDER_VK_INSTANCE_H_