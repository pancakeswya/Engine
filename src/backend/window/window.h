#ifndef BACKEND_WINDOW_H_
#define BACKEND_WINDOW_H_

#include <vulkan/vulkan.h>

#include <functional>
#include <memory>

namespace window {

enum class Type {
  kGlfw,
  kSdl
};

struct Size {
  int width;
  int height;
};

using ResizeCallback = std::function<void(void*, Size)>;

class EventHandler {
public:
  virtual ~EventHandler() = default;
  virtual void OnRenderEvent() = 0;
};

class Window {
public:
  using Handle = std::unique_ptr<Window>;

  virtual ~Window() = default;

  [[nodiscard]] virtual bool ShouldClose() const noexcept = 0;
  virtual void Loop(EventHandler* handler) const = 0;

  virtual void SetWindowResizedCallback(ResizeCallback resize_callback) = 0;
  virtual void SetWindowUserPointer(void* user_ptr) = 0;

  [[nodiscard]] virtual Size GetSize() const noexcept = 0;
};

namespace vk {

class SurfaceFactory {
public:
  virtual ~SurfaceFactory() = default;
  [[nodiscard]] virtual VkSurfaceKHR CreateSurface(VkInstance instance,
                                                   const VkAllocationCallbacks *allocator) const = 0;
};

class Window : public virtual window::Window {
public:
  virtual void WaitUntilResized() const = 0;

  [[nodiscard]] virtual std::vector<const char*> GetExtensions() const = 0;
  [[nodiscard]] virtual const SurfaceFactory& GetSurfaceFactory() const noexcept = 0;
};

} // namespace vk

namespace gl { class Window : public virtual window::Window {}; }

} // namespace window

#endif // BACKEND_WINDOW_H_
