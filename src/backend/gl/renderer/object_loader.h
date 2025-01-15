#ifndef BACKEND_GL_RENDERER_OBJECT_LOADER_H_
#define BACKEND_GL_RENDERER_OBJECT_LOADER_H_

#include "backend/gl/renderer/handle.h"
#include "backend/gl/renderer/object.h"

#include <string>

namespace gl {

class ObjectLoader {
public:
  explicit ObjectLoader(const ValueObject& program);
  ~ObjectLoader() = default;

  [[nodiscard]] Object Load(const std::string& path) const;
private:
  const ValueObject& program_;
};

inline ObjectLoader::ObjectLoader(const ValueObject& program) : program_(program) {}

} // namespace gl

#endif // BACKEND_GL_RENDERER_OBJECT_LOADER_H_
