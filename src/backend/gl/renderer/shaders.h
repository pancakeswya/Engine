#ifndef BACKEND_GL_RENDERER_SHADERS_H_
#define BACKEND_GL_RENDERER_SHADERS_H_

#include <string_view>
#include <vector>

namespace gl {

struct Shader {
  static std::vector<Shader> GetShaders();

  std::string_view code;
  int stage;
};

} // namespace gl

#endif // BACKEND_GL_RENDERER_SHADERS_H_
