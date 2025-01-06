#include "backend/window/provider.h"

#include <utility>

#include "backend/window/glfw/instance.h"
#include "backend/window/glfw/window.h"
#include "backend/window/sdl/instance.h"
#include "backend/window/sdl/window.h"

namespace window {

template <typename InstanceType, typename WindowType>
  void Provider::Provide(const Size size, const std::string& title) {
  instance_ = InstanceType::Init();
  window_ = std::make_unique<WindowType>(WindowType(size, title));
}

Provider::Provider(const BackendType type, const Size size, const std::string& title) {
  if (type == BackendType::kGlfw) {
    Provide<glfw::Instance, glfw::Window>(size, title);
  } else if (type == BackendType::kSdl) {
    Provide<sdl::Instance, sdl::Window>(size, title);
  }
}

} // namespace window