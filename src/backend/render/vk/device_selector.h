#ifndef BACKEND_RENDER_VK_SELECTOR_H_
#define BACKEND_RENDER_VK_SELECTOR_H_

#include <vector>
#include <utility>
#include <optional>

#include <vulkan/vulkan.h>

#include "backend/render/vk/device.h"

namespace render::vk {

class DeviceSelector {
public:
  struct Requirements {
    bool present;
    bool graphic;
    bool anisotropy;

    VkSurfaceKHR surface;

    std::vector<const char*> extensions;
  };

  explicit DeviceSelector(const std::vector<VkPhysicalDevice>& devices) noexcept;
  ~DeviceSelector() = default;

  [[nodiscard]] std::optional<Device> Select(const Requirements& requirements) const;
private:
  const std::vector<VkPhysicalDevice>& devices_;
};

inline DeviceSelector::DeviceSelector(const std::vector<VkPhysicalDevice>& devices) noexcept : devices_(devices) {}

} // namespace render::vk

#endif // BACKEND_RENDER_VK_SELECTOR_H_
