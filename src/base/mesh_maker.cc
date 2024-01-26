#include "base/mesh_maker.h"
#include "base/data_parser.h"

#include <map>

namespace engine::MeshMaker {

namespace {

struct compare {
  bool operator()(const Index& lhs, const Index& rhs) const noexcept {
    if (lhs.fv < rhs.fv) return true;
    if (rhs.fv < lhs.fv) return false;
    if (lhs.fn < rhs.fn) return true;
    if (rhs.fn < lhs.fn) return false;
    if (lhs.ft < rhs.ft) return true;
    return rhs.ft < lhs.ft;
  }
};

using IndexMap = std::map<Index, unsigned int, compare>;

void DataToMesh(Data* data, Mesh* mesh) {
  IndexMap index_map;

  mesh->vertices.reserve(data->indices.size());
  mesh->indices.reserve(data->indices.size());

  mesh->usemtl = std::move(data->usemtl);
  mesh->mtl = std::move(data->mtl);
  // create vertex for each unique index
  unsigned int next_combined_idx = 0, combined_idx = 0;
  for (const Index& idx : data->indices) {
    if (index_map.count(idx)) {
      combined_idx = index_map.at(idx);
    } else {
      combined_idx = next_combined_idx;
      index_map.insert({idx, combined_idx});

      unsigned int i_v = idx.fv * 3, i_n = idx.fn * 3, i_t = idx.ft * 2;

      mesh->vertices.push_back(data->v[i_v]);
      mesh->vertices.push_back(data->v[i_v + 1]);
      mesh->vertices.push_back(data->v[i_v + 2]);

      mesh->vertices.push_back(data->vt[i_t]);
      mesh->vertices.push_back(data->vt[i_t + 1]);

      mesh->vertices.push_back(data->vn[i_n]);
      mesh->vertices.push_back(data->vn[i_n + 1]);
      mesh->vertices.push_back(data->vn[i_n + 2]);

      ++next_combined_idx;
    }
    mesh->indices.push_back(combined_idx);
  }
  mesh->tex_coords = std::move(data->vt);
}

}  // namespace

std::pair<Mesh*, Status> FromFile(std::string_view path) {
  auto mesh = new Mesh;
  auto [data, stat] = DataParser::FromFile(path);
  if (stat == Status::kNoExc) {
    DataToMesh(data, mesh);
  }
  delete data;
  return {mesh, stat};
}

}  // namespace engine::MeshMaker
