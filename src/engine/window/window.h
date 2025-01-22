#ifndef ENGINE_WINDOW_WINDOW_H_
#define ENGINE_WINDOW_WINDOW_H_

#include <functional>
#include <memory>

namespace engine {

class Window {
public:
  using Handle = std::unique_ptr<Window, void(*)(Window*)>;

  class EventHandler {
  public:
    virtual ~EventHandler() = default;
    virtual void OnRenderEvent() = 0;
  };

  using ResizeCallback = std::function<void(int, int)>;

  virtual ~Window() = default;

  [[nodiscard]] virtual bool ShouldClose() const noexcept = 0;
  virtual void Loop() const = 0;

  virtual void SetWindowTitle(const std::string& title) = 0;
  virtual void SetWindowEventHandler(EventHandler* handler) = 0;
  virtual void SetWindowResizedCallback(ResizeCallback resize_callback) = 0;

  [[nodiscard]] virtual int GetWidth() const noexcept = 0;
  [[nodiscard]] virtual int GetHeight() const noexcept = 0;
};

} // namespace engine

#endif // ENGINE_WINDOW_WINDOW_H_
