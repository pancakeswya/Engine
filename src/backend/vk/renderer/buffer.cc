#include "backend/vk/renderer/buffer.h"

#include "backend/vk/renderer/error.h"

namespace vk {

Buffer::Buffer() noexcept : size_(0) {}

Buffer::Buffer(Buffer&& other) noexcept
  : Base(std::move(other)),
    memory_(std::move(other.memory_)),
    size_(other.size_) {
  other.size_ = 0;
}

Buffer::Buffer(DeviceDispatchable<VkBuffer>&& buffer,
               DeviceDispatchable<VkDeviceMemory>&& memory,
               const uint32_t size) noexcept
   : Base(std::move(buffer)),
     memory_(std::move(memory)),
     size_(size) {}

Buffer& Buffer::operator=(Buffer&& other) noexcept {
  if (this != &other) {
    Base::operator=(std::move(other));
    memory_ = std::move(other.memory_);
    size_ = std::exchange(other.size_, 0);
  }
  return *this;
}

void* Buffer::Map() const {
  void* data;
  if (const VkResult result = vkMapMemory(memory_.GetParent(), memory_.GetHandle(), 0, size_, 0, &data); result != VK_SUCCESS) {
    throw Error("failed to map buffer memory").WithCode(result);
  }
  return data;
}

void Buffer::Unmap() const noexcept {
  vkUnmapMemory(memory_.GetParent(), memory_.GetHandle());
}

uint32_t Buffer::Size() const noexcept {
  return size_;
}

} // namespace vk
