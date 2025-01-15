#ifndef ENGINE_RENDER_RENDERER_LOADER_H_
#define ENGINE_RENDER_RENDERER_LOADER_H_

#include "engine/render/renderer.h"
#include "engine/dll_loader.h"

namespace engine {

class RendererLoader final : DllLoader {
public:
  explicit RendererLoader(const std::string& path);

  [[nodiscard]] Renderer::Handle Load(Window& window) const;

  ~RendererLoader() override = default;
};

} // namespace engine

#endif // ENGINE_RENDER_RENDERER_LOADER_H_
