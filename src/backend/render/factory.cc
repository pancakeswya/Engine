#include "backend/render/factory.h"

#include "backend/render/vk/render.h"
#include "backend/render/gl/render.h"

namespace render {

Renderer::Handle Factory::CreateRenderer(window::Window& window) const {
  if (type_ == Type::kVk) {
    return Renderer::Handle(
      new vk::Renderer(vk::DefaultConfig(), dynamic_cast<window::vk::Window&>(window))
    );
  }
  return Renderer::Handle(
    new gl::Renderer(dynamic_cast<window::gl::Window&>(window))
  );
}

} // namespace render