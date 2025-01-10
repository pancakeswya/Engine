#ifndef BACKEND_RENDER_DATA_UTIL_H_
#define BACKEND_RENDER_DATA_UTIL_H_

#include "backend/render/types.h"
#include "obj/types.h"

namespace render::data_util {

extern void RemoveDuplicates(const obj::Data& data, Vertex* vertices, Index* indices);

} // namespace render

#endif // BACKEND_RENDER_DATA_UTIL_H_
