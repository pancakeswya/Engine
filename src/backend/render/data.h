#ifndef BACKEND_RENDER_DATA_H_
#define BACKEND_RENDER_DATA_H_

#include "backend/render/types.h"
#include "obj/types.h"

namespace render {

extern void RemoveDuplicatesFromData(const obj::Data& data, Vertex* vertices, Index* indices);

} // namespace render

#endif // BACKEND_RENDER_DATA_H_
