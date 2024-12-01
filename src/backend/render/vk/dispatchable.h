#ifndef BACKEND_RENDER_VK_DISPATCHABLE_H_
#define BACKEND_RENDER_VK_DISPATCHABLE_H_

#include <vulkan/vulkan.h>

#include <utility>

namespace vk {

template<typename HandleType, typename ParentWrapper>
class Dispatchable {
public:
  using ParentType = typename ParentWrapper::HandleType;
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
               DestroyFunc destroy,
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

  HandleType Handle() const noexcept { return handle_; }
  HandleType* HandlePtr() noexcept { return &handle_; }
  ParentType Parent() const noexcept { return parent_; }
protected:
  HandleType handle_;
  ParentType parent_;

  DestroyFunc destroy_;
  const VkAllocationCallbacks* allocator_;
private:
  void Destroy() noexcept {
    if (destroy_ != nullptr) {
      destroy_(parent_, handle_, allocator_);

      handle_ = VK_NULL_HANDLE;
      parent_ = VK_NULL_HANDLE;
      destroy_ = nullptr;
      allocator_ = VK_NULL_HANDLE;
    }
  }
};

} // namespace vk

#endif // BACKEND_RENDER_VK_DISPATCHABLE_H_