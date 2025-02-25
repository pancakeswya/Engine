#ifndef BACKEND_VK_RENDERER_HANDLE_H_
#define BACKEND_VK_RENDERER_HANDLE_H_

#include <utility>

#include <vulkan/vulkan.h>

namespace vk {

template<typename HandleType, typename CreatorType>
class NonDispatchableHandle {
public:
  using DestroyFunc = void (*)(CreatorType creator, HandleType type, const VkAllocationCallbacks*);

  NonDispatchableHandle() noexcept
    : handle_(VK_NULL_HANDLE),
      creator_(VK_NULL_HANDLE),
      destroy_(nullptr),
      allocator_(nullptr) {}

  NonDispatchableHandle(const NonDispatchableHandle& other) = delete;

  NonDispatchableHandle(NonDispatchableHandle&& other) noexcept
    : handle_(other.handle_),
      creator_(other.creator_),
      destroy_(other.destroy_),
      allocator_(other.allocator_) {
    other.handle_ = VK_NULL_HANDLE;
    other.creator_ = VK_NULL_HANDLE;
    other.destroy_ = nullptr;
    other.allocator_ = nullptr;
  }

  NonDispatchableHandle(HandleType handle,
               CreatorType creator,
               const DestroyFunc destroy,
               const VkAllocationCallbacks* allocator) noexcept
      : handle_(handle),
        creator_(creator),
        destroy_(destroy),
        allocator_(allocator) {}

  NonDispatchableHandle& operator=(const NonDispatchableHandle&) = delete;

  NonDispatchableHandle& operator=(NonDispatchableHandle&& other) noexcept {
    if (this != &other) {
      Destroy();
      handle_ = std::exchange(other.handle_, VK_NULL_HANDLE);
      creator_ = std::exchange(other.creator_, VK_NULL_HANDLE);
      destroy_ = std::exchange(other.destroy_, nullptr);
      allocator_ = std::exchange(other.allocator_, nullptr);
    }
    return *this;
  }

  virtual ~NonDispatchableHandle() { Destroy(); }

  [[nodiscard]] HandleType handle() const noexcept { return handle_; }
  [[nodiscard]] CreatorType creator() const noexcept { return creator_; }
  [[nodiscard]] const VkAllocationCallbacks* allocator() const noexcept { return allocator_; }
private:
  HandleType handle_;
  CreatorType creator_;
  DestroyFunc destroy_;
  const VkAllocationCallbacks* allocator_;

  void Destroy() noexcept {
    if (destroy_ != nullptr) {
      destroy_(creator_, handle_, allocator_);
      destroy_ = nullptr;
    }
  }
};

template<typename HandleType>
class Handle {
public:
  using DestroyFunc = void (*)(HandleType handle, const VkAllocationCallbacks*);

  Handle() noexcept : handle_(VK_NULL_HANDLE), destroy_(nullptr), allocator_(nullptr) {}

  Handle(const Handle&) = delete;

  Handle(Handle&& other) noexcept
    : handle_(other.handle_),
      destroy_(other.destroy_),
      allocator_(other.allocator_) {
    other.handle_ = VK_NULL_HANDLE;
    other.destroy_ = nullptr;
    other.allocator_ = VK_NULL_HANDLE;
  }

  Handle(HandleType handle,
                   const DestroyFunc destroy,
                   const VkAllocationCallbacks* allocator) noexcept
    : handle_(handle), destroy_(destroy), allocator_(allocator) {}

  virtual ~Handle() { Destroy(); }

  Handle& operator=(const Handle&) = delete;

  Handle& operator=(Handle&& other) noexcept {
    if (this != &other) {
      Destroy();
      handle_ = std::exchange(other.handle_, VK_NULL_HANDLE);
      destroy_ = std::exchange(other.destroy_, nullptr);
      allocator_ = std::exchange(other.allocator_, VK_NULL_HANDLE);
    }
    return *this;
  }

  [[nodiscard]] HandleType handle() const noexcept { return handle_; }
  [[nodiscard]] const VkAllocationCallbacks* allocator() const noexcept { return allocator_; }
private:
  HandleType handle_;
  DestroyFunc destroy_;
  const VkAllocationCallbacks* allocator_;

  void Destroy() noexcept {
    if (destroy_ != nullptr) {
      destroy_(handle_, allocator_);
      destroy_ = nullptr;
    }
  }
};

template<typename HandleType>
using DeviceHandle = NonDispatchableHandle<HandleType, VkDevice>;

template<typename HandleType>
using InstanceHandle = NonDispatchableHandle<HandleType, VkInstance>;

} // namespace vk

#endif // BACKEND_VK_RENDERER_HANDLE_H_