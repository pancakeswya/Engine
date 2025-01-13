#include "engine/data_util.h"

#include <glm/glm.hpp>
#include <unordered_map>

namespace engine::data_util {

void RemoveDuplicates(const obj::Data& data, Vertex* vertices, Index* indices) {
  std::unordered_map<obj::Indices, unsigned int, obj::Indices::Hash> index_map;

  unsigned int next_combined_idx = 0, combined_idx = 0;
  for (const obj::Indices& index : data.indices) {
    if (index_map.count(index)) {
      combined_idx = index_map.at(index);
    } else {
      combined_idx = next_combined_idx;
      index_map.emplace(index, combined_idx);
      unsigned int i_v = index.fv * 3, i_n = index.fn * 3, i_t = index.ft * 2;
      *vertices++ = Vertex{
        glm::vec3(data.v[i_v], data.v[i_v + 1], data.v[i_v + 2]),
        glm::vec3(data.vn[i_n], data.vn[i_n + 1], data.vn[i_n + 2]),
        glm::vec2(data.vt[i_t], data.vt[i_t + 1])
    };
      ++next_combined_idx;
    }
    *indices++ = combined_idx;
  }
}

} // namespace engine::data_util