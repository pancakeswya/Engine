#ifndef ENGINE_SRC_BASE_MESH_MAKER_H_
#define ENGINE_SRC_BASE_MESH_MAKER_H_

#include <string_view>
#include <utility>

#include "base/data_types.h"

namespace engine::MeshMaker {

extern std::pair<Mesh*, Status> FromFile(std::string_view path);

}  // namespace engine::MeshMaker

#endif  // ENGINE_SRC_BASE_MESH_MAKER_H_
