#include "backend/render/vk_buffer.h"
#include "backend/render/vk_factory.h"

namespace vk {

Buffer::Buffer(VkDevice logical_device, VkPhysicalDevice physical_device, VkBufferUsageFlags usage, uint32_t size)
  : size_(size),
    logical_device_(logical_device),
    physical_device_(physical_device),
    buffer_wrapper_(factory::CreateBuffer(logical_device, usage, size)) {}

void Buffer::Allocate(VkMemoryPropertyFlags properties) {
  memory_wrapper_ = factory::CreateBufferMemory(logical_device_, physical_device_, properties, buffer_wrapper_.get());
}

void Buffer::Bind() {
  if (const VkResult result = vkBindBufferMemory(logical_device_, buffer_wrapper_.get(), memory_wrapper_.get(), 0); result != VK_SUCCESS) {
    throw Error("failed to bind memory").WithCode(result);
  }
}

void* Buffer::Map() {
  void* data;
  if (const VkResult result = vkMapMemory(logical_device_, memory_wrapper_.get(), 0, size_, 0, &data); result != VK_SUCCESS) {
    throw Error("failed to map buffer memory").WithCode(result);
  }
  return data;
}

void Buffer::Unmap() noexcept {
  vkUnmapMemory(logical_device_, memory_wrapper_.get());
}

} // namespace vk