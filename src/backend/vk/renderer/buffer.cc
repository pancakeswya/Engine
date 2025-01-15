#include "backend/vk/renderer/buffer.h"

#include "backend/vk/renderer/error.h"
#include "backend/vk/renderer/internal.h"

namespace vk {

Buffer::Buffer() noexcept : physical_device_(VK_NULL_HANDLE), size_(0) {}

Buffer::Buffer(Buffer&& other) noexcept
  : Dispatchable(std::move(other)),
    memory_(std::move(other.memory_)),
    physical_device_(other.physical_device_),
    size_(other.size_) {
  other.physical_device_ = VK_NULL_HANDLE;
  other.size_ = 0;
}

Buffer::Buffer(VkBuffer buffer,
               VkDevice logical_device,
               VkPhysicalDevice physical_device,
               const VkAllocationCallbacks* allocator,
               const uint32_t size) noexcept
   : Dispatchable(buffer, logical_device, vkDestroyBuffer, allocator),
     physical_device_(physical_device),
     size_(size) {}


Buffer& Buffer::operator=(Buffer&& other) noexcept {
  if (this != &other) {
    static_cast<Base&>(*this) = static_cast<Base>(std::move(other));
    memory_ = std::move(other.memory_);
    physical_device_ = std::exchange(other.physical_device_, VK_NULL_HANDLE);
    size_ = std::exchange(other.size_, 0);
  }
  return *this;
}

void Buffer::Bind() const {
  if (const VkResult result = vkBindBufferMemory(parent_, handle_, memory_.Handle(), 0); result != VK_SUCCESS) {
    throw Error("failed to bind memory").WithCode(result);
  }
}

void Buffer::Allocate(const VkMemoryPropertyFlags properties) {
  VkMemoryRequirements mem_requirements;
  vkGetBufferMemoryRequirements(parent_, handle_, &mem_requirements);

  memory_ = internal::CreateMemory(parent_, physical_device_, allocator_, properties, mem_requirements);
}

void* Buffer::Map() const {
  void* data;
  if (const VkResult result = vkMapMemory(memory_.Parent(), memory_.Handle(), 0, size_, 0, &data); result != VK_SUCCESS) {
    throw Error("failed to map buffer memory").WithCode(result);
  }
  return data;
}

void Buffer::Unmap() const noexcept {
  vkUnmapMemory(memory_.Parent(), memory_.Handle());
}

uint32_t Buffer::Size() const noexcept {
  return size_;
}

} // namespace vk
