#ifndef ENGINE_SRC_MODEL_MESH_MODEL_H_
#define ENGINE_SRC_MODEL_MESH_MODEL_H_

#include <string_view>

namespace engine {

class Mesh;

class MeshModel {
 public:
  MeshModel() noexcept;
  ~MeshModel();

  const Mesh* GetMesh() noexcept;
  [[nodiscard]] bool CreateMesh(std::string_view path);

  void Reset() noexcept;
 private:
  Mesh* mesh_;
};

}  // namespace engine

#endif  // ENGINE_SRC_MODEL_MESH_MODEL_H_