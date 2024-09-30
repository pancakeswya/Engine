#ifndef BACKEND_RENDER_VK_WRAPPERS_H_
#define BACKEND_RENDER_VK_WRAPPERS_H_

#include "backend/render/vk_types.h"
#include "backend/render/vk_factory.h"

#include <vulkan/vulkan.h>

namespace vk {

#define DECL_MEMORY_OBJECT_CALLBACKS(NAME)                             \
template<>                                                             \
struct MemoryObjectCallbacks<Vk##NAME> {                               \
  static constexpr auto create_memory = factory::Create##NAME##Memory; \
  static constexpr auto bind_memory = vkBind##NAME##Memory;            \
}

template<typename Tp>
struct MemoryObjectCallbacks;

DECL_MEMORY_OBJECT_CALLBACKS(Buffer);
DECL_MEMORY_OBJECT_CALLBACKS(Image);

#undef DECL_MEMORY_OBJECT_CALLBACKS

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

  void TransitImageLayout(VkCommandPool cmd_pool, VkQueue graphics_queue, VkImageLayout old_layout, VkImageLayout new_layout);
  void CopyBuffer(const Buffer& src, VkCommandPool cmd_pool, VkQueue graphics_queue);

  [[nodiscard]] VkImageView GetView() const noexcept;
  [[nodiscard]] VkFormat GetFormat() const noexcept;
private:
  VkExtent2D extent_;
  VkFormat format_;

  HandleWrapper<VkImageView> view_;
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

inline Image::Image(VkDevice logical_device,
        VkExtent2D extent,
        uint32_t channels,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage) : extent_(extent), format_(format) {
  object_wrapper_ = factory::CreateImage(logical_device, extent, format, tiling, usage);
  logical_device_ = logical_device;
  size_ = extent.width * extent.height * channels;
}

inline void Image::CreateView(VkImageAspectFlags aspect_flags) {
  view_ = factory::CreateImageView(logical_device_, object_wrapper_.get(), format_, aspect_flags);
}

inline VkImageView Image::GetView() const noexcept {
  return view_.get();
}

inline VkFormat Image::GetFormat() const noexcept {
  return format_;
}

} // namespace vk

#endif // BACKEND_RENDER_VK_WRAPPERS_H_