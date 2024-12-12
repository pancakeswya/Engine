#ifndef BACKEND_WINDOW_H_
#define BACKEND_WINDOW_H_

#include <vulkan/vulkan.h>

#include <functional>

namespace window {

struct Size {
  int width;
  int height;
};

using ResizeCallback = std::function<void(void*, Size)>;

class ISurfaceFactory {
public:
  virtual ~ISurfaceFactory() = default;
  [[nodiscard]] virtual VkSurfaceKHR CreateSurface(VkInstance instance,
                                                   const VkAllocationCallbacks *allocator) const = 0;
};

class IWindow {
public:
  virtual ~IWindow() = default;

  virtual void WaitUntilResized() const = 0;
  [[nodiscard]] virtual bool ShouldClose() const = 0;
  virtual void HandleEvents() const = 0;

  virtual void SetWindowResizedCallback(ResizeCallback resize_callback) = 0;
  virtual void SetWindowUserPointer(void* user_ptr) = 0;

  [[nodiscard]] virtual Size GetSize() const noexcept = 0;
  [[nodiscard]] virtual std::vector<const char*> GetExtensions() const = 0;
  [[nodiscard]] virtual const ISurfaceFactory& GetSurfaceFactory() const noexcept = 0;
};

} // namespace window

#endif // BACKEND_WINDOW_H_
