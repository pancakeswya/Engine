#ifndef BACKEND_GL_RENDERER_OBJECT_LOADER_H_
#define BACKEND_GL_RENDERER_OBJECT_LOADER_H_

#include "backend/gl/renderer/handle.h"
#include "backend/gl/renderer/object.h"

#include <string>

namespace gl {

class ObjectLoader {
public:
  explicit ObjectLoader(const ValueHandle& program);
  ~ObjectLoader() = default;

  Object Load(const std::string& path);
private:
  const ValueHandle& program_;
};

inline ObjectLoader::ObjectLoader(const ValueHandle& program) : program_(program) {}

} // namespace gl

#endif // BACKEND_GL_RENDERER_OBJECT_LOADER_H_
