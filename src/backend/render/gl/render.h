#ifndef BACKEND_RENDER_GL_RENDER_H_
#define BACKEND_RENDER_GL_RENDER_H_

#include "backend/render/model.h"
#include "backend/render/render.h"
#include "obj/types.h"

#include <GL/glew.h>

#include <vector>

namespace window::gl {

class Window;

} // namespace window

namespace render::gl {

template<typename T>
struct DeleterInternal {
  using func = std::function<void(T)>;
};

template<typename T>
struct Deleter;

template<>
struct Deleter<GLuint> : DeleterInternal<GLuint> {};

template<>
struct Deleter<GLuint*> : DeleterInternal<GLuint*> {};

template<typename T>
class Handle {
public:
  Handle() = default;

  Handle(const Handle&) = delete;

  Handle(Handle&& other) noexcept : handle_(std::move(other.handle_)), deleter_(std::move(other.deleter_)) {
    other.handle_ = {};
    other.deleter_ = {};
  }

  explicit Handle(typename Deleter<T>::func deleter, const std::remove_pointer_t<T> handle = {}) noexcept
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

  Handle& operator=(Handle&& other) noexcept {
    if (this != &other) {
      handle_ = std::exchange(other.handle_, {});
      deleter_ = std::exchange(other.deleter_, {});
    }
    return *this;
  }

  [[nodiscard]] auto Value() const noexcept {
    return handle_;
  }
  [[nodiscard]] auto Ptr() noexcept { return &handle_; }
private:
  std::remove_pointer_t<T> handle_;
  typename Deleter<T>::func deleter_;
};

using ValueHandle = Handle<GLuint>;
using PointerHandle = Handle<GLuint*>;

struct Object {
  PointerHandle vbo;
  PointerHandle ebo;

  std::vector<PointerHandle> textures;
  std::vector<obj::UseMtl> usemtl;
};

class Renderer : public render::Renderer {
public:
  explicit Renderer(window::gl::Window& window);
  ~Renderer() override = default;

  void RenderFrame() override;
  void LoadModel(const std::string& path) override;
  [[nodiscard]] Model& GetModel() noexcept override;
private:
  window::gl::Window& window_;
  ValueHandle program_;

  Object object_;
  Model model_;
};

inline Model& Renderer::GetModel() noexcept {
  return model_;
}

} // namespace gl

#endif // BACKEND_RENDER_GL_RENDER_H_
