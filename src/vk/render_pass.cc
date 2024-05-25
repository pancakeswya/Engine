#include "vk/render_pass.h"
#include "vk/exception.h"

namespace vk {

RenderPass::RenderPass(
  VkDevice logical_device,
  VkFormat swap_chain_format
) : logical_device_(logical_device), pass_() {
  VkAttachmentDescription color_attachment = {};
  color_attachment.format = swap_chain_format;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref = {};
  color_attachment_ref .attachment = 0;
  color_attachment_ref .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref ;

  VkRenderPassCreateInfo render_pass_info = {};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;

  if (vkCreateRenderPass(logical_device, &render_pass_info, nullptr, &pass_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to create render pass");
  }
}

RenderPass::~RenderPass() {
  vkDestroyRenderPass(logical_device_, pass_, nullptr);
}

VkRenderPassBeginInfo RenderPass::BeginInfo(VkFramebuffer buffer, VkExtent2D extent, VkClearValue* clear_color) noexcept {
  VkRenderPassBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  begin_info.renderPass = pass_;
  begin_info.framebuffer = buffer;
  begin_info.renderArea.offset = {0, 0};
  begin_info.renderArea.extent = extent;
  begin_info.clearValueCount = 1;
  begin_info.pClearValues = clear_color;
  return begin_info;
}


} // namespace vk