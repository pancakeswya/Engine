#ifndef ENGINE_RENDERER_LOADER_H_
#define ENGINE_RENDERER_LOADER_H_

#include <string>

#include "engine/loader.h"
#include "engine/renderer.h"
#include "engine/renderer_entry.h"
#include "engine/window.h"
#include "window_entry.h"

namespace engine {

using RendererHandle = std::unique_ptr<Renderer, decltype(&DestroyRenderer)>;

class RendererLoader final : Loader {
public:
  explicit RendererLoader(const std::string& path);

  [[nodiscard]] RendererHandle LoadRenderer(Window& window) const;

  ~RendererLoader() override = default;
};

inline RendererLoader::RendererLoader(const std::string& path) : Loader(path) {}

inline RendererHandle RendererLoader::LoadRenderer(Window& window) const {
  const auto create_renderer = GetProc<decltype(&CreateRenderer)>("CreateRenderer");
  const auto destroy_renderer = GetProc<decltype(&DestroyRenderer)>("DestroyRenderer");
  return {create_renderer(window), destroy_renderer};
}

} // namespace engine

#endif // ENGINE_RENDERER_LOADER_H_
