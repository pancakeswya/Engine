#include "vk/framebuffer.h"
#include "vk/exception.h"

namespace vk {

Framebuffer::Framebuffer(
  VkDevice logical_device,
  VkRenderPass render_pass,
  VkImageView view,
  VkExtent2D extent
) : logical_device_(logical_device), framebuffer_() {
  VkFramebufferCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  create_info.renderPass = render_pass;
  create_info.attachmentCount = 1;
  create_info.pAttachments = &view;
  create_info.width = extent.width;
  create_info.height = extent.height;
  create_info.layers = 1;

  if (vkCreateFramebuffer(logical_device, &create_info, nullptr, &framebuffer_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to create framebuffer");
  }
}

Framebuffer::~Framebuffer() {
  vkDestroyFramebuffer(logical_device_, framebuffer_, nullptr);
}

} // namespace vk