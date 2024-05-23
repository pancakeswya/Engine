#ifndef VK_DEVICE_H_
#define VK_DEVICE_H_

#include <array>
#include <vulkan/vulkan.h>

namespace vk {

class Instance;
class Surface;

struct SwapChainSupportDetails;

namespace device {

namespace physical {

inline constexpr std::array kExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

bool ExtensionSupport(VkPhysicalDevice device);
SwapChainSupportDetails SwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
VkPhysicalDevice Find(VkInstance instance, VkSurfaceKHR surface);

} // namespace physical

class Logical {
 public:
  explicit Logical(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
  ~Logical();
  [[nodiscard]] VkDevice Get() const noexcept;
  [[nodiscard]] VkQueue GetGraphicsQueue() const noexcept;
  [[nodiscard]] VkQueue GetPresentQueue() const noexcept;

 private:
  VkDevice device_;
  VkQueue graphics_q_;
  VkQueue present_q_;
};

inline Logical::~Logical() { vkDestroyDevice(device_, nullptr); }

inline VkDevice Logical::Get() const noexcept { return device_; }

inline VkQueue Logical::GetGraphicsQueue() const noexcept { return graphics_q_; }

inline VkQueue Logical::GetPresentQueue() const noexcept { return present_q_; }

} // namespace device

} // namespace vk

#endif // VK_DEVICE_H_