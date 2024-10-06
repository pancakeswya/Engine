#ifndef BACKEND_RENDER_VK_WRAPPERS_H_
#define BACKEND_RENDER_VK_WRAPPERS_H_

#include "backend/render/vk_types.h"
#include "backend/render/vk_factory.h"

#include <vulkan/vulkan.h>

namespace vk {

template<typename Tp>
struct MemoryObjectCallbacks;

template <>
struct MemoryObjectCallbacks<VkBuffer> {
  static constexpr auto create_memory = factory::CreateBufferMemory;
  static constexpr auto bind_memory = vkBindBufferMemory;
};

template <>
struct MemoryObjectCallbacks<VkImage> {
  static constexpr auto create_memory = factory::CreateImageMemory;
  static constexpr auto bind_memory = vkBindImageMemory;
};

template<typename Tp>
class MemoryObject {
public:
  void Allocate(VkPhysicalDevice physical_device, VkMemoryPropertyFlags properties);
  void Bind();

  [[nodiscard]] Tp Get() const noexcept;
  [[nodiscard]] size_t Size() const noexcept;
protected:
  uint32_t size_;
  VkDevice logical_device_;

  HandleWrapper<VkDeviceMemory> memory_wrapper_;
  HandleWrapper<Tp> object_wrapper_;
};

class Buffer : public MemoryObject<VkBuffer> {
public:
  DECL_UNIQUE_OBJECT(Buffer);

  Buffer(VkDevice logical_device, VkBufferUsageFlags usage, uint32_t size);

  void* Map();
  void Unmap() noexcept;

  void CopyBuffer(const Buffer& src, VkCommandPool cmd_pool, VkQueue graphics_queue);
};

class Image : public MemoryObject<VkImage> {
public:
  DECL_UNIQUE_OBJECT(Image);

  Image(VkDevice logical_device,
        VkExtent2D extent,
        uint32_t channels,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage);

  void CreateView(VkImageAspectFlags aspect_flags);

  void GenerateMipmaps(VkPhysicalDevice physical_device, VkCommandPool cmd_pool, VkQueue graphics_queue);
  void TransitImageLayout(VkCommandPool cmd_pool, VkQueue graphics_queue, VkImageLayout old_layout, VkImageLayout new_layout);
  void CopyBuffer(const Buffer& src, VkCommandPool cmd_pool, VkQueue graphics_queue);

  [[nodiscard]] VkImageView GetView() const noexcept;
  [[nodiscard]] VkSampler GetSampler() const noexcept;
  [[nodiscard]] VkFormat GetFormat() const noexcept;
private:
  uint32_t mip_levels_;

  VkExtent2D extent_;
  VkFormat format_;

  HandleWrapper<VkImageView> view_;
  HandleWrapper<VkSampler> sampler_;
};

template<typename Tp>
inline void MemoryObject<Tp>::Allocate(VkPhysicalDevice physical_device, VkMemoryPropertyFlags properties) {
  memory_wrapper_ = MemoryObjectCallbacks<Tp>::create_memory(logical_device_, physical_device, properties, object_wrapper_.get());
}

template<typename Tp>
inline void MemoryObject<Tp>::Bind() {
  if (const VkResult result = MemoryObjectCallbacks<Tp>::bind_memory(logical_device_, object_wrapper_.get(), memory_wrapper_.get(), 0); result != VK_SUCCESS) {
    throw Error("failed to bind memory").WithCode(result);
  }
}

template<typename Tp>
inline Tp MemoryObject<Tp>::Get() const noexcept {
  return object_wrapper_.get();
}

template<typename Tp>
inline size_t MemoryObject<Tp>::Size() const noexcept {
  return size_;
}

inline Buffer::Buffer(VkDevice logical_device, VkBufferUsageFlags usage, uint32_t size) {
  object_wrapper_ = factory::CreateBuffer(logical_device, usage, size);
  logical_device_ = logical_device;
  size_ = size;
}

inline void* Buffer::Map() {
  void* data;
  if (const VkResult result = vkMapMemory(logical_device_, memory_wrapper_.get(), 0, size_, 0, &data); result != VK_SUCCESS) {
    throw Error("failed to map buffer memory").WithCode(result);
  }
  return data;
}

inline void Buffer::Unmap() noexcept {
  vkUnmapMemory(logical_device_, memory_wrapper_.get());
}

inline void Image::CreateView(VkImageAspectFlags aspect_flags) {
  view_ = factory::CreateImageView(logical_device_, object_wrapper_.get(), format_, aspect_flags, mip_levels_);
}

inline VkImageView Image::GetView() const noexcept {
  return view_.get();
}

inline VkFormat Image::GetFormat() const noexcept {
  return format_;
}

inline VkSampler Image::GetSampler() const noexcept {
  return sampler_.get();
}

} // namespace vk

#endif // BACKEND_RENDER_VK_WRAPPERS_H_