#include "backend/vk/renderer/commander.h"

#include "backend/vk/renderer/error.h"
#include "backend/vk/renderer/config.h"
#include "backend/vk/renderer/buffer.h"
#include "backend/vk/renderer/image.h"

namespace vk {

Commander::Commander(VkDevice logical_device, VkCommandPool cmd_pool, VkQueue graphics_queue)
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

Commander::~Commander() {
  vkFreeCommandBuffers(logical_device_, cmd_pool_, 1, &cmd_buffer_);
}

void Commander::Begin() const {
  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  if (const VkResult result = vkBeginCommandBuffer(cmd_buffer_, &begin_info); result != VK_SUCCESS) {
    throw Error("failed to begin command buffer").WithCode(result);
  }
}

void Commander::End() const {
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
  : Commander(buffer.Parent(), cmd_pool, graphics_queue), buffer_(buffer) {}


void BufferCommander::CopyBuffer(const Buffer& src) const {
  VkBufferCopy copy_region = {};
  copy_region.size = src.Size();
  vkCmdCopyBuffer(cmd_buffer_, src.Handle(), buffer_.Handle(), 1, &copy_region);
}

ImageCommander::ImageCommander(Image& image, VkCommandPool cmd_pool, VkQueue graphics_queue)
    : Commander(image.Parent(), cmd_pool, graphics_queue), image_(image) {}

void ImageCommander::GenerateMipmaps() const {
  VkImage image = image_.Handle();

  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = image;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;

  auto mip_width = static_cast<int32_t>(image_.Extent().width);
  auto mip_height = static_cast<int32_t>(image_.Extent().height);

  for (uint32_t i = 1; i < image_.MipLevels(); i++) {
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

  barrier.subresourceRange.baseMipLevel = image_.MipLevels() - 1;
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
  barrier.image = image_.Handle();
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = image_.MipLevels();
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

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
  region.imageExtent = { image_.Extent().width, image_.Extent().height, 1 };

  vkCmdCopyBufferToImage(cmd_buffer_, src.Handle(), image_.Handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

} // namespace vk