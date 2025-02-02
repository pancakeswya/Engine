#include "engine/render/renderer_loader.h"

#include "engine/render/plugin.h"

namespace engine {

RendererLoader::RendererLoader(const std::string& path) : DllLoader(path) {}

Renderer::Handle RendererLoader::Load(Window& window) const {
  const auto create_renderer = DllLoader::Load<decltype(&PluginCreateRenderer)>("PluginCreateRenderer");
  const auto destroy_renderer = DllLoader::Load<decltype(&PluginDestroyRenderer)>("PluginDestroyRenderer");
  return {create_renderer(window), destroy_renderer};
}

} // namespace engine