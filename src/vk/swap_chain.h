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

  struct Images {
    VkFormat format;
    std::vector<VkImage> images;
    std::vector<VkImageView> views;
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

  Images& GetImages() noexcept;
  VkExtent2D GetExtent() noexcept;
  VkSwapchainKHR GetChain() noexcept;
private:
  Images images_;
  VkExtent2D extent_;
  VkDevice logical_device_;
  VkSwapchainKHR swapchain_;
};

inline SwapChain::Images& SwapChain::GetImages() noexcept {
  return images_;
}

inline VkExtent2D SwapChain::GetExtent() noexcept {
  return extent_;
}

inline VkSwapchainKHR SwapChain::GetChain() noexcept {
  return swapchain_;
}

} // namespace vk

#endif // VK_SWAP_CHAIN_H_