#include "vk/swap_chain.h"
#include "vk/queue.h"
#include "vk/exception.h"

#include <algorithm>
#include <limits>

namespace vk {

namespace {

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats) {
  for (const auto& available_format : available_formats) {
    if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return available_format;
    }
  }
  return available_formats[0];
}

VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes) {
  for (const auto& available_present_mode : available_present_modes) {
    if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return available_present_mode;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  VkExtent2D actualExtent = {
    static_cast<uint32_t>(width),
    static_cast<uint32_t>(height)
  };
  actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
  actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

  return actualExtent;
}


} // namespace

SwapChain::SupportDetails SwapChain::Support(
  VkPhysicalDevice device,
  VkSurfaceKHR surface
) {
  SupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);

  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count,
                                         details.formats.data());
  }

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                            &present_mode_count, nullptr);

  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &present_mode_count, details.present_modes.data());
  }
  return details;
}

SwapChain::SwapChain(
  GLFWwindow* window,
  VkPhysicalDevice physical_device,
  VkDevice logical_device,
  VkSurfaceKHR surface
) : images_(), extent_(), logical_device_(logical_device), swapchain_() {
  SupportDetails support = Support(physical_device, surface);

  VkSurfaceFormatKHR surface_format = ChooseSwapSurfaceFormat(support.formats);
  VkPresentModeKHR present_mode = ChooseSwapPresentMode(support.present_modes);
  VkExtent2D extent = ChooseSwapExtent(window, support.capabilities);

  uint32_t image_count = support.capabilities.minImageCount + 1;
  if (support.capabilities.maxImageCount > 0 && image_count > support.capabilities.maxImageCount) {
    image_count = support.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = surface;

  create_info.minImageCount = image_count;
  create_info.imageFormat = surface_format.format;
  create_info.imageColorSpace = surface_format.colorSpace;
  create_info.imageExtent = extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  auto [_, indices] = queue::FindFamilyIndices(physical_device, surface);
  uint32_t queueFamilyIndices[] = {indices.graphic, indices.present};

  if (indices.graphic != indices.present) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  create_info.preTransform = support.capabilities.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;

  create_info.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(logical_device, &create_info, nullptr, &swapchain_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to create swap chain!");
  }
  vkGetSwapchainImagesKHR(logical_device, swapchain_, &image_count, nullptr);
  images_.images.resize(image_count);
  vkGetSwapchainImagesKHR(logical_device, swapchain_, &image_count, images_.images.data());

  images_.format = surface_format.format;
  extent_ = extent;
}

SwapChain::~SwapChain() {
  for (auto view : images_.views) {
    vkDestroyImageView(logical_device_, view, nullptr);
  }
  vkDestroySwapchainKHR(logical_device_, swapchain_, nullptr);
}

} // namespace vk