#ifndef ENGINE_SRC_VIEW_RENDERER_H_
#define ENGINE_SRC_VIEW_RENDERER_H_

#include "base/data_types.h"

namespace engine {

class MeshModel;

class Renderer {
 public:
  explicit Renderer(MeshModel* model);
  Result Render();
 private:
  MeshModel* model_;
};

} // namespace engine

#endif // ENGINE_SRC_VIEW_RENDERER_H_