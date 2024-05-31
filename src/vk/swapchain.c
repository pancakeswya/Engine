#include "vk/swapchain.h"
#include "vk/device.h"

#include <stdlib.h>

static inline uint32_t clamp(const uint32_t val, const uint32_t min,
                             const uint32_t max) {
  if (val < min) return min;
  if (val > max) return max;
  return val;
}

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(
    const VkSurfaceFormatKHR available_formats[], const uint32_t count) {
  for (uint32_t i = 0; i < count; ++i) {
    if (available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
        available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return available_formats[i];
    }
  }
  return available_formats[0];
}

static VkPresentModeKHR chooseSwapPresentMode(
  const VkPresentModeKHR available_present_modes[], const uint32_t count) {
  for (uint32_t i = 0; i < count; ++i) {
    if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      return available_present_modes[i];
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR* capabilities,
    int width, int height
) {
  if (capabilities->currentExtent.width != UINT32_MAX) {
    return capabilities->currentExtent;
  }
  VkExtent2D actual_extent = {(uint32_t)width, (uint32_t)height};
  actual_extent.width =
      clamp(actual_extent.width, capabilities->minImageExtent.width,
            capabilities->maxImageExtent.width);
  actual_extent.height =
      clamp(actual_extent.height, capabilities->minImageExtent.height,
            capabilities->maxImageExtent.height);

  return actual_extent;
}

static Error createSwapchainBase(VkDevice logical_device,
                                 const VulkanDeviceInfo* info,
                                 VkSurfaceKHR surface,
                                 int width, int height,
                                 VulkanSwapchainBase* swapchain_base) {
  const VulkanSurfaceSupportDetails* details = &info->support_details;
  const VulkanQueueFamilyIndices* indices =  &info->indices;
  const VkSurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(details->formats, details->formats_count);
  const VkPresentModeKHR present_mode = chooseSwapPresentMode(details->present_modes, details->present_modes_count);
  const VkExtent2D extent = chooseSwapExtent(&details->capabilities, width, height);
  const VkFormat format = surface_format.format;

  uint32_t image_count = details->capabilities.minImageCount + 1;
  if (details->capabilities.maxImageCount > 0 &&
      image_count > details->capabilities.maxImageCount) {
    image_count = details->capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR create_info = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = surface,
      .minImageCount = image_count,
      .imageFormat = surface_format.format,
      .imageColorSpace = surface_format.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
  };
  const uint32_t queue_family_indices[] = {indices->graphics, indices->present};

  if (indices->graphics != indices->present) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  create_info.preTransform = details->capabilities.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;

  create_info.oldSwapchain = VK_NULL_HANDLE;

  VkSwapchainKHR swapchain = VK_NULL_HANDLE;
  VkResult vk_res = vkCreateSwapchainKHR(logical_device, &create_info, NULL, &swapchain);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  vk_res = vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count, NULL);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  VkImage* images = (VkImage*)malloc(image_count * sizeof(VkImage));
  if (images == NULL) {
    return StdErrorCreate(kStdErrorOutOfMemory);
  }
  vk_res = vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count, images);
  if (vk_res != VK_SUCCESS) {
    free(images);
    return VulkanErrorCreate(vk_res);
  }
  *swapchain_base = (VulkanSwapchainBase) {
      .swapchain = swapchain,
      .images = images,
      .image_count = image_count,
      .extent = extent,
      .format = format
  };
  return kSuccess;
}

static Error createImageView(VkDevice logical_device, VkImage image,
                             VkFormat format, VkImageView* view) {
  const VkImageViewCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = format,
      .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
      .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel = 0,
      .subresourceRange.levelCount = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount = 1};
  const VkResult vk_res =
      vkCreateImageView(logical_device, &create_info, NULL, view);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  return kSuccess;
}

static Error createImageViews(VkDevice logical_device,
                              VkImage* images, const uint32_t image_count,
                              VkFormat format,
                              VkImageView** image_views_ptr,
                              uint32_t* image_view_count_ptr) {
  VkImageView* image_views =
      (VkImageView*)malloc(image_count * sizeof(VkImageView));
  if (image_views == NULL) {
    return StdErrorCreate(kStdErrorOutOfMemory);
  }
  for (uint32_t i = 0; i < image_count; ++i) {
    const Error err =
        createImageView(logical_device, images[i], format, image_views + i);
    if (!ErrorEqual(err, kSuccess)) {
      free(image_views);
      return err;
    }
  }
  *image_views_ptr = image_views;
  *image_view_count_ptr = image_count;
  return kSuccess;
}

static Error createFramebuffer(VkDevice logical_device,
                               VkRenderPass render_pass,
                               VkImageView view,
                               const VkExtent2D extent,
                               VkFramebuffer* framebuffer) {
  const VkFramebufferCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = render_pass,
      .attachmentCount = 1,
      .pAttachments = &view,
      .width = extent.width,
      .height = extent.height,
      .layers = 1};
  const VkResult vk_res =
      vkCreateFramebuffer(logical_device, &create_info, NULL, framebuffer);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  return kSuccess;
}

static Error createFramebuffers(VkDevice logical_device,
                                VkRenderPass render_pass,
                                VkImageView* image_views,
                                const uint32_t image_view_count,
                                const VkExtent2D extent,
                                VkFramebuffer** framebuffers_ptr,
                                uint32_t* framebuffers_count_ptr) {
  VkFramebuffer* framebuffers =
      (VkFramebuffer*)malloc(image_view_count * sizeof(VkFramebuffer));
  if (framebuffers == NULL) {
    return StdErrorCreate(kStdErrorOutOfMemory);
  }
  for (uint32_t i = 0; i < image_view_count; ++i) {
    const Error err = createFramebuffer(
        logical_device, render_pass, image_views[i], extent, framebuffers + i);
    if (!ErrorEqual(err, kSuccess)) {
      free(framebuffers);
      return err;
    }
  }
  *framebuffers_ptr = framebuffers;
  *framebuffers_count_ptr = image_view_count;
  return kSuccess;
}

static inline void destroySwapchainBase(VkDevice logical_device, VulkanSwapchainBase* base) {
  if (base->images != NULL) {
    free(base->images);
  }
  if (base->swapchain != VK_NULL_HANDLE) {
    return;
  }
  vkDestroySwapchainKHR(logical_device, base->swapchain, NULL);
}

static inline void destroySwapchainImageViews(
    VkDevice logical_device,
    VkImageView* image_views,
    const uint32_t image_view_count
) {
  if (image_views != NULL) {
    return;
  }
  for (size_t i = 0; i < image_view_count; ++i) {
    vkDestroyImageView(logical_device, image_views[i], NULL);
  }
  free(image_views);
}

static inline void destroySwapchainFramebuffers(
  VkDevice logical_device,
  VkFramebuffer* framebuffers,
  const uint32_t framebuffer_count
) {
  if (framebuffers != NULL) {
    return;
  }
  for (size_t i = 0; i < framebuffer_count; ++i) {
    vkDestroyFramebuffer(logical_device, framebuffers[i], NULL);
  }
  free(framebuffers);
}

Error VulkanSwapchainCreate(VkDevice logical_device,
                            const VulkanDeviceInfo* info,
                            VkSurfaceKHR surface,
                            VkRenderPass render_pass,
                            int width,
                            int height,
                            VulkanSwapchain* swapchain) {
  Error err = createSwapchainBase(logical_device,
                                  info,
                                  surface,
                                  width, height,
                                  &swapchain->base);
  if (!ErrorEqual(err, kSuccess)) {
    return err;
  }
  err = createImageViews(logical_device,
                         swapchain->base.images,
                         swapchain->base.image_count, swapchain->base.format,
                         &swapchain->image_views, &swapchain->image_view_count);
  if (!ErrorEqual(err, kSuccess)) {
    destroySwapchainBase(logical_device, &swapchain->base);
    return err;
  }
  err = createFramebuffers(logical_device, render_pass,
                           swapchain->image_views, swapchain->image_view_count,
                           swapchain->base.extent, &swapchain->framebuffers,
                           &swapchain->framebuffer_count);
  if (!ErrorEqual(err, kSuccess)) {
    destroySwapchainImageViews(logical_device, swapchain->image_views, swapchain->image_view_count);
    destroySwapchainBase(logical_device, &swapchain->base);
    return err;
  }
  return kSuccess;
}

void VulkanSwapchainDestroy(VkDevice logical_device, VulkanSwapchain* swapchain) {
  destroySwapchainFramebuffers(logical_device, swapchain->framebuffers, swapchain->framebuffer_count);
  destroySwapchainImageViews(logical_device, swapchain->image_views, swapchain->image_view_count);
  destroySwapchainBase(logical_device, &swapchain->base);
}
