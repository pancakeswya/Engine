#include "backend/vk/renderer/swapchain.h"

#include <algorithm>
#include <limits>

#include "backend/vk/renderer/internal.h"
#include "backend/vk/renderer/error.h"

namespace vk {

namespace {

VkFormat FindSupportedFormat(const std::vector<VkFormat>& formats, VkPhysicalDevice physical_device, VkImageTiling tiling, VkFormatFeatureFlags features) {
  for (const VkFormat format : formats) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);
    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    }
    if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw Error("failed to find supported format");
}

inline VkFormat FindDepthFormat(VkPhysicalDevice physical_device) {
  return FindSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      physical_device,
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
  );
}

} // namespace

VkSurfaceFormatKHR Swapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats) {
  for (const VkSurfaceFormatKHR& available_format : available_formats) {
    if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return available_format;
        }
  }
  return available_formats[0];
}

VkPresentModeKHR Swapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& available_present_modes) {
  for (const auto& available_present_mode : available_present_modes) {
    if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return available_present_mode;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::ChooseExtent(const VkExtent2D extent, const VkSurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }

  return {
    std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
    std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
  };
}

Swapchain::Swapchain() noexcept : extent_(), format_(VK_FORMAT_UNDEFINED), physical_device_(VK_NULL_HANDLE)  {}

Swapchain::Swapchain(Swapchain&& other) noexcept
  : Dispatchable(std::move(other)),
    extent_(other.extent_),
    format_(other.format_),
    physical_device_(other.physical_device_),
    depth_image_(std::move(other.depth_image_)),
    image_views_(std::move(other.image_views_)) {
  other.physical_device_ = VK_NULL_HANDLE;
  other.extent_ = {};
  other.format_ = {};
}

Swapchain& Swapchain::operator=(Swapchain&& other) noexcept {
  if (this != &other) {
    static_cast<Base&>(*this) = static_cast<Base>(std::move(other));
    physical_device_ = std::exchange(other.physical_device_, VK_NULL_HANDLE);
    extent_ = std::exchange(other.extent_, {});
    format_ = std::exchange(other.format_, {});
    depth_image_ = std::move(other.depth_image_);
    image_views_ = std::move(other.image_views_);
  }
  return *this;
}

Swapchain::Swapchain(VkSwapchainKHR swapchain,
                     VkDevice logical_device,
                     VkPhysicalDevice physical_device,
                     const VkAllocationCallbacks* allocator,
                     const VkExtent2D extent,
                     const VkFormat format) noexcept :
      Dispatchable(swapchain, logical_device, vkDestroySwapchainKHR, allocator),
      extent_(extent),
      format_(format),
      physical_device_(physical_device),
      depth_image_(CreateDepthImage()),
      image_views_(CreateImageViews()) {}

std::vector<VkImage> Swapchain::GetImages() const {
  uint32_t image_count;
  if (const VkResult result = vkGetSwapchainImagesKHR(parent_, handle_, &image_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get swapchain image count").WithCode(result);
  }
  std::vector<VkImage> images(image_count);
  if (const VkResult result = vkGetSwapchainImagesKHR(parent_, handle_, &image_count, images.data()); result != VK_SUCCESS) {
    throw Error("failed to get swapchain images").WithCode(result);
  }
  return images;
}

std::vector<Device::Dispatchable<VkImageView>> Swapchain::CreateImageViews() const {
  std::vector<VkImage> images = GetImages();
  std::vector<Dispatchable<VkImageView>> image_views;
  image_views.reserve(images.size());
  for(VkImage image : images) {
    Dispatchable<VkImageView> image_view = internal::CreateImageView(image, parent_, allocator_, VK_IMAGE_ASPECT_COLOR_BIT, format_);
    image_views.emplace_back(std::move(image_view));
  }
  return image_views;
}

Image Swapchain::CreateDepthImage() const {
  const VkFormat depth_format = FindDepthFormat(physical_device_);

  Image depth_image = internal::CreateImage(parent_, physical_device_, allocator_, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, extent_, depth_format, VK_IMAGE_TILING_OPTIMAL);
  depth_image.Allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  depth_image.Bind();
  depth_image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);
  return depth_image;
}

Device::Dispatchable<VkFramebuffer> Swapchain::CreateFramebuffer(const std::vector<VkImageView>& views, VkRenderPass render_pass) const {
  VkFramebufferCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  create_info.renderPass = render_pass;
  create_info.attachmentCount = static_cast<uint32_t>(views.size());
  create_info.pAttachments = views.data();
  create_info.width = extent_.width;
  create_info.height = extent_.height;
  create_info.layers = 1;

  VkFramebuffer framebuffer = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateFramebuffer(parent_, &create_info, allocator_, &framebuffer); result != VK_SUCCESS) {
    throw Error("failed to create framebuffer").WithCode(result);
  }
  return {
    framebuffer,
    parent_,
    vkDestroyFramebuffer,
    allocator_,
  };
}

std::vector<Device::Dispatchable<VkFramebuffer>> Swapchain::CreateFramebuffers(VkRenderPass render_pass) const {
  std::vector<Dispatchable<VkFramebuffer>> framebuffers;
  framebuffers.reserve(image_views_.size());
  for(const Dispatchable<VkImageView>& image_view : image_views_) {
    Dispatchable<VkFramebuffer> framebuffer = CreateFramebuffer({image_view.Handle(), depth_image_.View().Handle()}, render_pass);
    framebuffers.emplace_back(std::move(framebuffer));
  }
  return framebuffers;
}

} // namespace vk
