#include "engine/render/renderer.h"

#include "engine/render/plugin.h"

namespace engine {

Renderer::Loader::Loader(const std::string& path) : DllLoader(path) {}

Renderer::Handle Renderer::Loader::Load(Window& window) const {
  const auto create_renderer = DllLoader::Load<decltype(&CreateRenderer)>("CreateRenderer");
  const auto destroy_renderer = DllLoader::Load<decltype(&DestroyRenderer)>("DestroyRenderer");
  return {create_renderer(window), destroy_renderer};
}

} // namespace engine