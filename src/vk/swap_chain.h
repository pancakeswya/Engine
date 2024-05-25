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

  VkImageView Get() noexcept;
private:
  VkDevice logical_device_;
  VkImageView view_;
};

inline VkImageView ImageView::Get() noexcept {
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

  VkSwapchainKHR Get() noexcept;
  std::vector<VkImage> Images() noexcept;
  VkExtent2D Extent() noexcept;
  VkFormat Format() noexcept;
private:
  VkDevice logical_device_;
  VkExtent2D extent_;
  VkFormat format_;
  VkSwapchainKHR swapchain_;
};

inline VkSwapchainKHR SwapChain::Get() noexcept {
  return swapchain_;
}

inline VkExtent2D SwapChain::Extent() noexcept {
  return extent_;
}

inline VkFormat SwapChain::Format() noexcept {
  return format_;
}

} // namespace vk

#endif // VK_SWAP_CHAIN_H_