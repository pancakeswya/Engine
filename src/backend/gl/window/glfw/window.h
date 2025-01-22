#ifndef BACKEND_GL_WINDOW_GLFW_WINDOW_H_
#define BACKEND_GL_WINDOW_GLFW_WINDOW_H_

#include "backend/gl/renderer/window.h"
#include "backend/internal/glfw/window.h"

#include <string>

namespace glfw::gl {

class Window final : public internal::Window, public ::gl::Window {
public:
  Window(int width, int height, const std::string& title);
  ~Window() override = default;

  void Loop() const override;
  void SetWindowEventHandler(EventHandler* handler) noexcept override;
};

} // namespace glfw::gl

#endif // BACKEND_GL_WINDOW_GLFW_WINDOW_H_