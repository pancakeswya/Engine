#include "view/renderer.h"
#include "model/mesh_model.h"

#include <GL/gl.h>

namespace engine {

Renderer::Renderer(MeshModel* model) : model_(model) {}

Result Renderer::Render() {
  constexpr std::string_view path = "C:/Users/niyaz/Pictures/OBJ/tommy";
  if (!model_->CreateMesh(path)) {
    return kFailure;
  }
  auto mesh = model_->GetMesh();
  glClear(GL_COLOR_BUFFER_BIT);
  delete mesh;
  return kSuccess;
}

} // namespace engine