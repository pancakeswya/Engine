#ifndef VK_SWAP_CHAIN_H_
#define VK_SWAP_CHAIN_H_

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vk {

class SwapChain {
public:
  struct SupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
  };

  static SupportDetails Support(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
  );

  SwapChain(
    GLFWwindow* window,
    VkPhysicalDevice physical_device,
    VkDevice logical_device,
    VkSurfaceKHR surface
  );
  ~SwapChain();

  VkSwapchainKHR GetChain() noexcept;
  VkExtent2D GetExtent() noexcept;
  VkFormat GetFormat() noexcept;
private:
  VkDevice logical_device_;
  VkExtent2D extent_;
  VkFormat format_;
  VkSwapchainKHR swapchain_;
};

inline VkSwapchainKHR SwapChain::GetChain() noexcept {
  return swapchain_;
}

inline VkExtent2D SwapChain::GetExtent() noexcept {
  return extent_;
}

inline VkFormat SwapChain::GetFormat() noexcept {
  return format_;
}


} // namespace vk

#endif // VK_SWAP_CHAIN_H_