#include "backend/render/vk_commander.h"
#include "backend/render/vk_wrappers.h"
#include "backend/render/vk_config.h"
#include "backend/render/vk_object.h"

namespace vk {

SingleTimeCommander::SingleTimeCommander(VkDevice logical_device, VkCommandPool cmd_pool, VkQueue graphics_queue)
  : logical_device_(logical_device),
    cmd_pool_(cmd_pool),
    cmd_buffer_(VK_NULL_HANDLE),
    graphics_queue_(graphics_queue) {
  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandPool = cmd_pool;
  alloc_info.commandBufferCount = 1;

  if (const VkResult result = vkAllocateCommandBuffers(logical_device, &alloc_info, &cmd_buffer_); result != VK_SUCCESS) {
    throw Error("failed to allocate command buffers").WithCode(result);
  }
}

SingleTimeCommander::~SingleTimeCommander() {
  vkFreeCommandBuffers(logical_device_, cmd_pool_, 1, &cmd_buffer_);
}

void SingleTimeCommander::Begin() const {
  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  if (const VkResult result = vkBeginCommandBuffer(cmd_buffer_, &begin_info); result != VK_SUCCESS) {
    throw Error("failed to begin command buffer").WithCode(result);
  }
}

void SingleTimeCommander::End() const {
  if (const VkResult result = vkEndCommandBuffer(cmd_buffer_); result != VK_SUCCESS) {
    throw Error("failed to end cmd buffer").WithCode(result);
  }

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmd_buffer_;

  if (const VkResult result = vkQueueSubmit(graphics_queue_, 1, &submitInfo, VK_NULL_HANDLE); result != VK_SUCCESS) {
    throw Error("failed to submin the graphics queue");
  }
  if (const VkResult result = vkQueueWaitIdle(graphics_queue_); result != VK_SUCCESS) {
    throw Error("failed to wait idle graphics queue").WithCode(result);
  }
}

BufferCommander::BufferCommander(Buffer& buffer, VkCommandPool cmd_pool, VkQueue graphics_queue)
  : SingleTimeCommander(buffer.logical_device_, cmd_pool, graphics_queue), buffer_(buffer) {}


void BufferCommander::CopyBuffer(const Buffer& src) const {
  VkBufferCopy copy_region = {};
  copy_region.size = src.Size();
  vkCmdCopyBuffer(cmd_buffer_, src.Get(), buffer_.Get(), 1, &copy_region);
}

ImageCommander::ImageCommander(Image& image, VkCommandPool cmd_pool, VkQueue graphics_queue)
    : SingleTimeCommander(image.logical_device_, cmd_pool, graphics_queue), image_(image) {}

void ImageCommander::GenerateMipmaps() const {
  VkImage image = image_.image_wrapper_.get();

  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = image;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;

  auto mip_width = static_cast<int32_t>(image_.extent_.width);
  auto mip_height = static_cast<int32_t>(image_.extent_.height);

  for (uint32_t i = 1; i < image_.mip_levels_; i++) {
      barrier.subresourceRange.baseMipLevel = i - 1;
      barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

      vkCmdPipelineBarrier(cmd_buffer_,
          VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
          0, nullptr,
          0, nullptr,
          1, &barrier);

      VkImageBlit blit = {};
      blit.srcOffsets[0] = {0, 0, 0};
      blit.srcOffsets[1] = {mip_width, mip_height, 1};
      blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      blit.srcSubresource.mipLevel = i - 1;
      blit.srcSubresource.baseArrayLayer = 0;
      blit.srcSubresource.layerCount = 1;
      blit.dstOffsets[0] = {0, 0, 0};
      blit.dstOffsets[1] = { mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
      blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      blit.dstSubresource.mipLevel = i;
      blit.dstSubresource.baseArrayLayer = 0;
      blit.dstSubresource.layerCount = 1;

      vkCmdBlitImage(cmd_buffer_,
          image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
          image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          1, &blit,
          VK_FILTER_LINEAR);

      barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      vkCmdPipelineBarrier(cmd_buffer_,
          VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
          0, nullptr,
          0, nullptr,
          1, &barrier);

      if (mip_width > 1) mip_width /= 2;
      if (mip_height > 1) mip_height /= 2;
  }

  barrier.subresourceRange.baseMipLevel = image_.mip_levels_ - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(cmd_buffer_,
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
      0, nullptr,
      0, nullptr,
      1, &barrier);
}

void ImageCommander::TransitImageLayout(VkImageLayout old_layout, VkImageLayout new_layout) const {
  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image_.image_wrapper_.get();
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = image_.mip_levels_;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;

  VkPipelineStageFlags source_stage;
  VkPipelineStageFlags destination_stage;

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    throw Error("unsupported layout transition");
  }
  vkCmdPipelineBarrier(
      cmd_buffer_,
      source_stage, destination_stage,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier
  );
}

void ImageCommander::CopyBuffer(const Buffer& src) const {
  VkBufferImageCopy region = {};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = { image_.extent_.width, image_.extent_.height, 1 };

  vkCmdCopyBufferToImage(cmd_buffer_, src.Get(), image_.image_wrapper_.get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

RenderCommander::RenderCommander(VkDevice logical_device, VkCommandPool cmd_pool)
  : curr_buffer_(), cmd_buffers_(factory::CreateCommandBuffers(logical_device, cmd_pool, config::kFrameCount)) {}

void RenderCommander::Next() noexcept {
  curr_buffer_ = (curr_buffer_ + 1) % config::kFrameCount;
}

void RenderCommander::Begin() const {
  VkCommandBuffer cmd_buffer = cmd_buffers_[curr_buffer_];
  if (const VkResult result = vkResetCommandBuffer(cmd_buffer, 0); result != VK_SUCCESS) {
    throw Error("failed to reset command buffer").WithCode(result);
  }
  VkCommandBufferBeginInfo cmd_buffer_begin_info = {};
  cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  if (const VkResult result = vkBeginCommandBuffer(cmd_buffer, &cmd_buffer_begin_info); result != VK_SUCCESS) {
    throw Error("failed to begin recording command buffer").WithCode(result);
  }
}

void RenderCommander::BeginRender(VkRenderPass render_pass, VkFramebuffer framebuffer, VkPipeline pipeline, VkExtent2D extent) const {
  std::array<VkClearValue, 2> clear_values = {};
  clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  clear_values[1].depthStencil = {1.0f, 0};

  VkRenderPassBeginInfo render_pass_begin_info = {};
  render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_begin_info.renderPass = render_pass;
  render_pass_begin_info.framebuffer = framebuffer;
  render_pass_begin_info.renderArea.offset = {0, 0};
  render_pass_begin_info.renderArea.extent = extent;
  render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
  render_pass_begin_info.pClearValues = clear_values.data();

  VkCommandBuffer cmd_buffer = cmd_buffers_[curr_buffer_];
  vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(extent.width);
  viewport.height = static_cast<float>(extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

  VkRect2D scissor = {};
  scissor.offset = {0, 0};
  scissor.extent = extent;
  vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);
}

void RenderCommander::EndRender() const {
  vkCmdEndRenderPass(cmd_buffers_[curr_buffer_]);
}

void RenderCommander::End() const {
  if (const VkResult result = vkEndCommandBuffer(cmd_buffers_[curr_buffer_]); result != VK_SUCCESS) {
    throw Error("failed to record command buffer").WithCode(result);
  }
}

void RenderCommander::DrawObject(Object& object, VkPipelineLayout pipeline_layout, VkDescriptorSet descriptor_set) const {
  VkCommandBuffer cmd_buffer = cmd_buffers_[curr_buffer_];

  VkBuffer vertices_buffer = object.vertices.Get();
  VkBuffer indices_buffer = object.indices.Get();

  VkDeviceSize prev_offset = 0;
  std::array<VkDeviceSize, 1> vertex_offsets = {};

  for(const auto[index, offset] : object.usemtl) {
    const VkDeviceSize curr_offset = prev_offset * sizeof(Index::type);

    vkCmdPushConstants(cmd_buffer, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(unsigned int), &index);
    vkCmdBindVertexBuffers(cmd_buffer, 0, vertex_offsets.size(), &vertices_buffer, vertex_offsets.data());
    vkCmdBindIndexBuffer(cmd_buffer, indices_buffer, curr_offset, Index::type_enum);
    vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);
    vkCmdDrawIndexed(cmd_buffer, static_cast<uint32_t>(offset - prev_offset), 1, 0, 0, 0);

    prev_offset = offset;
  }
}

void RenderCommander::Submit(VkQueue graphics_queue, VkFence fence, VkSemaphore wait_semaphore, VkSemaphore signal_semaphore) const {
  const std::vector<VkPipelineStageFlags> pipeline_stage_flags = config::GetPipelineStageFlags();

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &wait_semaphore;
  submit_info.pWaitDstStageMask = pipeline_stage_flags.data();
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = cmd_buffers_.data() + curr_buffer_;
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &signal_semaphore;

  if (const VkResult result = vkQueueSubmit(graphics_queue, 1, &submit_info, fence); result != VK_SUCCESS) {
    throw Error("failed to submit draw command buffer").WithCode(result);
  }
}

} // namespace vk