#ifndef BACKEND_VK_RENDERER_DISPATCHABLE_H_
#define BACKEND_VK_RENDERER_DISPATCHABLE_H_

#include <utility>

#include <vulkan/vulkan.h>

namespace vk {

template<typename HandleType, typename ParentType>
class Dispatchable {
public:
  using DestroyFunc = void (*)(ParentType parent, HandleType type, const VkAllocationCallbacks*);

  Dispatchable() noexcept
    : handle_(VK_NULL_HANDLE),
      parent_(VK_NULL_HANDLE),
      destroy_(nullptr),
      allocator_(VK_NULL_HANDLE) {}

  Dispatchable(const Dispatchable&) = delete;

  Dispatchable(Dispatchable&& other) noexcept
    : handle_(other.handle_),
      parent_(other.parent_),
      destroy_(other.destroy_),
      allocator_(other.allocator_) {
    other.handle_ = VK_NULL_HANDLE;
    other.parent_ = VK_NULL_HANDLE;
    other.destroy_ = nullptr;
    other.allocator_ = VK_NULL_HANDLE;
  }

  Dispatchable(HandleType handle,
               ParentType parent,
               const DestroyFunc destroy,
               const VkAllocationCallbacks* allocator) noexcept
      : handle_(handle),
        parent_(parent),
        destroy_(destroy),
        allocator_(allocator) {}

  Dispatchable& operator=(const Dispatchable&) = delete;

  Dispatchable& operator=(Dispatchable&& other) noexcept {
    if (this != &other) {
      Destroy();
      handle_ = std::exchange(other.handle_, VK_NULL_HANDLE);
      parent_ = std::exchange(other.parent_, VK_NULL_HANDLE);
      destroy_ = std::exchange(other.destroy_, nullptr);
      allocator_ = std::exchange(other.allocator_, VK_NULL_HANDLE);
    }
    return *this;
  }

  virtual ~Dispatchable() { Destroy(); }

  [[nodiscard]] HandleType GetHandle() const noexcept { return handle_; }
  [[nodiscard]] const VkAllocationCallbacks* GetAllocator() const noexcept { return allocator_; }
protected:
  HandleType handle_;
  ParentType parent_;

  DestroyFunc destroy_;
  const VkAllocationCallbacks* allocator_;
private:
  void Destroy() noexcept {
    if (destroy_ != nullptr) {
      if (handle_ != VK_NULL_HANDLE) {
        destroy_(parent_, handle_, allocator_);
        handle_ = VK_NULL_HANDLE;
      }
      parent_ = VK_NULL_HANDLE;
      destroy_ = nullptr;
      allocator_ = VK_NULL_HANDLE;
    }
  }
};

template<typename HandleType>
class SelfDispatchable {
public:
  using DestroyFunc = void (*)(HandleType handle, const VkAllocationCallbacks*);

  SelfDispatchable() noexcept
      : handle_(VK_NULL_HANDLE), destroy_(nullptr), allocator_(nullptr) {}

  SelfDispatchable(const SelfDispatchable&) = delete;

  SelfDispatchable(SelfDispatchable&& other) noexcept
    : handle_(other.handle_),
      destroy_(other.destroy_),
      allocator_(other.allocator_) {
    other.handle_ = VK_NULL_HANDLE;
    other.destroy_ = nullptr;
    other.allocator_ = VK_NULL_HANDLE;
  }

  SelfDispatchable(HandleType handle,
                   const DestroyFunc destroy,
                   const VkAllocationCallbacks* allocator) noexcept
    : handle_(handle), destroy_(destroy), allocator_(allocator) {}

  virtual ~SelfDispatchable() { Destroy(); }

  SelfDispatchable& operator=(const SelfDispatchable&) = delete;

  SelfDispatchable& operator=(SelfDispatchable&& other) noexcept {
    if (this != &other) {
      Destroy();
      handle_ = std::exchange(other.handle_, VK_NULL_HANDLE);
      destroy_ = std::exchange(other.destroy_, nullptr);
      allocator_ = std::exchange(other.allocator_, VK_NULL_HANDLE);
    }
    return *this;
  }

  [[nodiscard]] HandleType GetHandle() const noexcept { return handle_; }
  [[nodiscard]] const VkAllocationCallbacks* GetAllocator() const noexcept { return allocator_; }
protected:
  HandleType handle_;
  DestroyFunc destroy_;
  const VkAllocationCallbacks* allocator_;
private:
  void Destroy() noexcept {
    if (destroy_ != nullptr) {
      if (handle_ != VK_NULL_HANDLE) {
        destroy_(handle_, allocator_);
        handle_ = VK_NULL_HANDLE;
      }
      destroy_ = nullptr;
      allocator_ = VK_NULL_HANDLE;
    }
  }
};

template<typename HandleType>
class InstanceDispatchable : public Dispatchable<HandleType, VkInstance> {
public:
  using Base = Dispatchable<HandleType, VkInstance>;
  using Base::Base;

  [[nodiscard]] VkInstance GetInstance() const noexcept { return Base::parent_; }
};

template<typename HandleType>
class DeviceDispatchable : public Dispatchable<HandleType, VkDevice> {
public:
  using Base = Dispatchable<HandleType, VkDevice>;
  using Base::Base;

  [[nodiscard]] VkDevice GetDevice() const noexcept { return Base::parent_; }
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_DISPATCHABLE_H_