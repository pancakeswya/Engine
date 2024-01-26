#include "model/mesh_model.h"
#include "base/mesh_maker.h"

namespace engine {

MeshModel::MeshModel() noexcept : mesh_() {}

MeshModel::~MeshModel() { delete mesh_; }

bool MeshModel::CreateMesh(std::string_view path) {
  auto [mesh, status] = MeshMaker::FromFile(path);
  if (status != Status::kNoExc) {
    // log or handle
    return false;
  }
  mesh_ = mesh;
  return true;
}

const Mesh* MeshModel::GetMesh() noexcept { return mesh_; }

void MeshModel::Reset() noexcept {
  delete mesh_;
  mesh_ = nullptr;
}

}  // namespace objv