#ifndef BACKEND_VK_RENDERER_PHYSICAL_DEVICE_H_
#define BACKEND_VK_RENDERER_PHYSICAL_DEVICE_H_

#include <vector>

#include <vulkan/vulkan.h>

namespace vk {

class PhysicalDevice final {
public:
  struct SurfaceSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
  };

  explicit PhysicalDevice(VkPhysicalDevice physical_device) noexcept;
  PhysicalDevice() = default;

  [[nodiscard]] VkPhysicalDevice handle() const noexcept;

  [[nodiscard]] SurfaceSupportDetails surface_support_details(VkSurfaceKHR surface) const;
  [[nodiscard]] bool format_feature_supported(VkFormat format, VkFormatFeatureFlagBits feature) const;
  [[nodiscard]] int32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] VkFormat FindSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features) const;
  [[nodiscard]] bool extensions_support(const std::vector<const char*>& extensions) const;
  [[nodiscard]] std::vector<VkQueueFamilyProperties> queue_family_properties() const;
  [[nodiscard]] VkBool32 surface_supported(VkSurfaceKHR surface, uint32_t queue_family_idx) const;
  [[nodiscard]] VkPhysicalDeviceFeatures features() const;
private:
  VkPhysicalDevice physical_device_;
};

inline PhysicalDevice::PhysicalDevice(VkPhysicalDevice physical_device) noexcept
  : physical_device_(physical_device) {}

inline VkPhysicalDevice PhysicalDevice::handle() const noexcept {
  return physical_device_;
}

} // namespace vk

#endif // BACKEND_VK_RENDERER_PHYSICAL_DEVICE_H_
