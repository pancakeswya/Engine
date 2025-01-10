#include "backend/window/factory.h"

#include "backend/window/glfw/vk/instance.h"
#include "backend/window/glfw/vk/window.h"
#include "backend/window/glfw/gl/instance.h"
#include "backend/window/glfw/gl/window.h"

#include "backend/window/sdl/vk/instance.h"
#include "backend/window/sdl/vk/window.h"

namespace window {

namespace vk {

Instance::Handle Factory::CreateInstance() const {
  if (type_ == Type::kGlfw) {
    return glfw::vk::Instance::Init();
  }
  return sdl::vk::Instance::Init();
}

Window::Handle Factory::CreateWindow(const Size size, const std::string& title) const {
  if (type_ == Type::kGlfw) {
    return Window::Handle(new glfw::vk::Window(size, title));
  }
  return Window::Handle(new sdl::vk::Window(size, title));
}

} // namespace vk

namespace gl {

Instance::Handle Factory::CreateInstance() const {
  if (type_ == Type::kGlfw) {
    return Instance::Handle(glfw::gl::Instance::Init());
  }
  return {};
}

Window::Handle Factory::CreateWindow(const Size size, const std::string& title) const {
  if (type_ == Type::kGlfw) {
    return Window::Handle(new glfw::gl::Window(size, title));
  }
  return {};
}

} // namespace gl

} // namespace window