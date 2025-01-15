#ifndef BACKEND_GL_RENDERER_OBJECT_H_
#define BACKEND_GL_RENDERER_OBJECT_H_

#include <GL/glew.h>

#include "backend/gl/renderer/handle.h"
#include "obj/types.h"

namespace gl {

struct Object {
  ArrayHandle vbo;
  ArrayHandle ebo;

  std::vector<ArrayHandle> textures;
  std::vector<obj::UseMtl> usemtl;
};

} // namespace gl

#endif // BACKEND_GL_RENDERER_OBJECT_H_
