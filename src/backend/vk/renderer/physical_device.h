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

  [[nodiscard]] VkPhysicalDevice GetHandle() const noexcept;

  [[nodiscard]] SurfaceSupportDetails GetSurfaceSupportDetails(VkSurfaceKHR surface) const;
  [[nodiscard]] bool GetFormatFeatureSupported(VkFormat format, VkFormatFeatureFlagBits feature) const;
  [[nodiscard]] int32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const;
  [[nodiscard]] VkFormat FindSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features) const;
  [[nodiscard]] bool GetExtensionsSupport(const std::vector<const char*>& extensions) const;
  [[nodiscard]] std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties() const;
  [[nodiscard]] VkBool32 GetSurfaceSupported(VkSurfaceKHR surface, uint32_t queue_family_idx) const;
  [[nodiscard]] VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures() const;
private:
  VkPhysicalDevice physical_device_;
};

inline PhysicalDevice::PhysicalDevice(VkPhysicalDevice physical_device) noexcept
  : physical_device_(physical_device) {}

inline VkPhysicalDevice PhysicalDevice::GetHandle() const noexcept {
  return physical_device_;
}

} // namespace vk

#endif // BACKEND_VK_RENDERER_PHYSICAL_DEVICE_H_
