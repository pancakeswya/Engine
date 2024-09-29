#ifndef BACKEND_RENDER_VK_WRAPPERS_H_
#define BACKEND_RENDER_VK_WRAPPERS_H_

#include "backend/render/vk_factory.h"

#include <vulkan/vulkan.h>

namespace vk {

#define DECL_MEMORY_OBJECT(NAME)           \
  NAME() = default;                        \
  NAME(const NAME&) = delete;              \
  NAME(NAME&&) = default;                  \
  ~NAME() = default;                       \
  NAME& operator=(const NAME&) = delete;   \
  NAME& operator=(NAME&&) = default

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

template<typename Tp>
class MemoryObject {
public:
  DECL_MEMORY_OBJECT(MemoryObject);

  MemoryObject(HandleWrapper<Tp>&& object_wrapper, VkDevice logical_device, uint32_t size)
    : size_(size), logical_device_(logical_device), object_wrapper_(std::move(object_wrapper)) {}

  void Allocate(VkPhysicalDevice physical_device, VkMemoryPropertyFlags properties) {
    memory_wrapper_ = MemoryObjectCallbacks<Tp>::create_memory(logical_device_, physical_device, properties, object_wrapper_.get());
  }

  void Bind() {
    if (const VkResult result = MemoryObjectCallbacks<Tp>::bind_memory(logical_device_, object_wrapper_.get(), memory_wrapper_.get(), 0); result != VK_SUCCESS) {
      throw Error("failed to bind memory").WithCode(result);
    }
  }

  [[nodiscard]] Tp Get() const noexcept {
    return object_wrapper_.get();
  }
protected:
  uint32_t size_;
  VkDevice logical_device_;

  HandleWrapper<VkDeviceMemory> memory_wrapper_;
  HandleWrapper<Tp> object_wrapper_;
};

class Buffer : public MemoryObject<VkBuffer> {
public:
  DECL_MEMORY_OBJECT(Buffer);

  Buffer(VkDevice logical_device, VkBufferUsageFlags usage, uint32_t size) {
    HandleWrapper<VkBuffer> buffer = factory::CreateBuffer(logical_device, usage, size);
    static_cast<MemoryObject&>(*this) = MemoryObject(std::move(buffer), logical_device, size);
  }

  void* Map() {
    void* data;
    if (const VkResult result = vkMapMemory(logical_device_, memory_wrapper_.get(), 0, size_, 0, &data); result != VK_SUCCESS) {
      throw Error("failed to map buffer memory").WithCode(result);
    }
    return data;
  }

  void Unmap() noexcept {
    vkUnmapMemory(logical_device_, memory_wrapper_.get());
  }
};

class Image : public MemoryObject<VkImage> {
public:
  DECL_MEMORY_OBJECT(Image);

  Image(VkDevice logical_device,
        uint32_t width,
        uint32_t height,
        uint32_t channels,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage) : format_(format) {
    HandleWrapper<VkImage> image = factory::CreateImage(logical_device, width, height, format, tiling, usage);
    static_cast<MemoryObject&>(*this) = MemoryObject(std::move(image), logical_device, width * height * channels);
  }

  void CreateView(VkImageAspectFlags aspect_flags) {
    view_ = factory::CreateImageView(logical_device_, object_wrapper_.get(), format_, aspect_flags);
  }

  [[nodiscard]] VkImageView GetView() const noexcept {
    return view_.get();
  }

  [[nodiscard]] VkFormat GetFormat() const noexcept {
    return format_;
  }
private:
  VkFormat format_;
  HandleWrapper<VkImageView> view_;
};

#undef DECL_MEMORY_OBJECT
#undef DECL_MEMORY_OBJECT_CALLBACKS

} // namespace vk

#endif // BACKEND_RENDER_VK_WRAPPERS_H_