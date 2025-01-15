#ifndef BACKEND_GL_RENDERER_HANDLE_H_
#define BACKEND_GL_RENDERER_HANDLE_H_

#include <GL/glew.h>

#include <functional>
#include <type_traits>
#include <utility>

namespace gl {

template<typename T>
class Handle {
public:
  using Deleter = std::function<void(T)>;
  using Value = std::remove_pointer_t<T>;
  using Ptr = Value*;

  Handle() = default;

  Handle(const Handle&) = delete;

  Handle(Handle&& other) noexcept : handle_(std::move(other.handle_)), deleter_(std::move(other.deleter_)) {
    other.handle_ = {};
    other.deleter_ = {};
  }

  explicit Handle(Value handle, Deleter deleter) noexcept
    : handle_(handle), deleter_(std::move(deleter)) {}

  ~Handle() {
    if (deleter_) {
      if constexpr (std::is_pointer_v<T>) {
        deleter_(&handle_);
      } else {
        deleter_(handle_);
      }
    }
  }

  Handle& operator=(const Handle&) = delete;

  Handle& operator=(Handle&& other) noexcept {
    if (this != &other) {
      handle_ = std::exchange(other.handle_, {});
      deleter_ = std::exchange(other.deleter_, {});
    }
    return *this;
  }

  [[nodiscard]] Value GetValue() const noexcept {
    return handle_;
  }
private:
  Value handle_;
  Deleter deleter_;
};

using ValueHandle = Handle<GLuint>;
using ArrayHandle = Handle<GLuint*>;

} // namespace gl

#endif // BACKEND_GL_RENDERER_HANDLE_H_
