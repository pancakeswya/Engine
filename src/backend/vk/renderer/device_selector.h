#ifndef BACKEND_VK_RENDERER_DEVICE_SELECTOR_H_
#define BACKEND_VK_RENDERER_DEVICE_SELECTOR_H_

#include <vector>
#include <optional>

#include <vulkan/vulkan.h>

#include "backend/vk/renderer/device.h"

namespace vk {

class DeviceSelector {
public:
  struct Requirements {
    bool present;
    bool graphic;
    bool anisotropy;

    VkSurfaceKHR surface;

    std::vector<const char*> extensions;
  };

  explicit DeviceSelector(const std::vector<VkPhysicalDevice>& physical_devices) noexcept;
  ~DeviceSelector() = default;

  [[nodiscard]] std::optional<Device> Select(const Requirements& requirements, const VkAllocationCallbacks* allocator = nullptr) const;
private:
  const std::vector<VkPhysicalDevice>& physical_devices_;
};

inline DeviceSelector::DeviceSelector(const std::vector<VkPhysicalDevice>& physical_devices) noexcept : physical_devices_(physical_devices) {}

} // namespace vk

#endif // BACKEND_VK_RENDERER_DEVICE_SELECTOR_H_
