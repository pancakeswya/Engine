#ifndef ENGINE_RENDER_RENDERER_H_
#define ENGINE_RENDER_RENDERER_H_

#include "engine/render/model.h"
#include "engine/window/window.h"
#include "engine/dll_loader.h"

namespace engine {

class Renderer {
public:
  using Handle = std::unique_ptr<Renderer, void(*)(Renderer*)>;

  class Loader final : DllLoader {
  public:
    explicit Loader(const std::string& path);

    [[nodiscard]] Handle Load(Window& window) const;

    ~Loader() override = default;
  };

  virtual void RenderFrame() = 0;
  virtual void LoadModel(const std::string& path) = 0;
  virtual Model& GetModel() noexcept = 0;
  virtual ~Renderer() = default;
};

} // namespace engine

#endif // ENGINE_RENDER_RENDERER_H_
