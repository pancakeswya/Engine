#include "vk/command.h"
#include "vk/devices.h"
#include "vk/exception.h"

namespace vk::command {

Pool::Pool(
  VkDevice logical_device,
  VkPhysicalDevice physical_device,
  VkSurfaceKHR surface
) : logical_device_(logical_device), pool_() {
  auto[_, family_indices] = queue::FindFamilyIndices(physical_device, surface);

  VkCommandPoolCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  create_info.queueFamilyIndex = family_indices.graphic;
  create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(logical_device_, &create_info, nullptr, &pool_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to create command pool");
  }
}

Pool::~Pool() {
  vkDestroyCommandPool(logical_device_, pool_, nullptr);
}

Buffers::Buffers(VkDevice logical_device, VkCommandPool pool, uint32_t count) : buffers_() {
  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = count;
  buffers_ = static_cast<VkCommandBuffer*>(
    ::operator new(count * sizeof(VkCommandBuffer))
  );
  if (vkAllocateCommandBuffers(logical_device, &alloc_info, buffers_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to allocate command buffers");
  }
}

Buffers::~Buffers() {
  ::operator delete(buffers_);
}


Record::Record(VkCommandBuffer buffer) : buffer_(buffer) {
  vkResetCommandBuffer(buffer_,0);
}

void Record::Begin() {
  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(buffer_, &begin_info) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to begin recording command buffer");
  }
}

void Record::End() {
  if (vkEndCommandBuffer(buffer_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to record command buffer");
  }
}

} // namespace vk::command