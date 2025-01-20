#ifndef BACKEND_VK_RENDERER_INSTANCE_DISPATCHABLE_FACTORY_H_
#define BACKEND_VK_RENDERER_INSTANCE_DISPATCHABLE_FACTORY_H_

#include "backend/vk/renderer/instance.h"
#include "backend/vk/renderer/window.h"

namespace vk {

class InstanceDispatchableFactory {
public:
  explicit InstanceDispatchableFactory(const Instance& instance) noexcept;

#ifdef DEBUG
  [[nodiscard]] InstanceDispatchable<VkDebugUtilsMessengerEXT> CreateMessenger() const;
#endif
  [[nodiscard]] InstanceDispatchable<VkSurfaceKHR> CreateSurface(const Window& window) const;
private:
  const Instance& instance_;
};

inline InstanceDispatchableFactory::InstanceDispatchableFactory(const Instance& instance) noexcept
  : instance_(instance) {}

} // namespace vk

#endif // BACKEND_VK_RENDERER_INSTANCE_DISPATCHABLE_FACTORY_H_
