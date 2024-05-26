#ifndef VK_SWAP_CHAIN_H_
#define VK_SWAP_CHAIN_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

namespace vk {

class ImageView {
public:
  ImageView(
    VkDevice logical_device,
    VkImage image,
    VkFormat format
  );
  ~ImageView();

  VkImageView get() noexcept;
private:
  VkDevice logical_device_;
  VkImageView view_;
};

inline VkImageView ImageView::get() noexcept {
  return view_;
}

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

  VkSwapchainKHR get_swapchain() noexcept;
  std::vector<VkImage> get_images() noexcept;
  VkExtent2D get_extent() noexcept;
  VkFormat get_format() noexcept;
private:
  VkDevice logical_device_;
  VkExtent2D extent_;
  VkFormat format_;
  VkSwapchainKHR swapchain_;
  std::vector<VkImage> images_;
};

inline VkSwapchainKHR SwapChain::get_swapchain() noexcept {
  return swapchain_;
}

inline VkExtent2D SwapChain::get_extent() noexcept {
  return extent_;
}

inline VkFormat SwapChain::get_format() noexcept {
  return format_;
}

inline std::vector<VkImage> SwapChain::get_images() noexcept {
  return images_;
}

} // namespace vk

#endif // VK_SWAP_CHAIN_H_