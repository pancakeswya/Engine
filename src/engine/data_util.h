#ifndef ENGINE_DATA_UTIL_H_
#define ENGINE_DATA_UTIL_H_

#include "engine/types.h"
#include "obj/types.h"

namespace engine::data_util {

extern void RemoveDuplicates(const obj::Data& data, Vertex* vertices, Index* indices);

} // namespace engine

#endif // ENGINE_DATA_UTIL_H_
