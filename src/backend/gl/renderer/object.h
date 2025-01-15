#ifndef BACKEND_GL_RENDERER_OBJECT_H_
#define BACKEND_GL_RENDERER_OBJECT_H_

#include <GL/glew.h>

#include "backend/gl/renderer/handle_object.h"
#include "obj/types.h"

namespace gl {

struct Object {
  ArrayObject vbo;
  ArrayObject ebo;

  std::vector<ArrayObject> textures;
  std::vector<obj::UseMtl> usemtl;
};

} // namespace gl

#endif // BACKEND_GL_RENDERER_OBJECT_H_
