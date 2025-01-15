#include "backend/gl/renderer/renderer.h"

#include <vector>

#include <glm/gtc/type_ptr.hpp>

#include "engine/render/data_util.h"
#include "backend/gl/renderer/error.h"
#include "backend/gl/renderer/shaders.h"
#include "backend/gl/renderer/object_loader.h"

namespace gl {

namespace {

void InitLoaderOnce() {
  static bool initialized = false;
  if (!initialized) {
    if (glewInit() != GLEW_OK) {
      throw Error("Failed to gl loader");
    }
    initialized = true;
  }
}

inline void CompileShader(const char* source, const GLuint shader) {
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    throw Error("Failed to compile shader").WithMessage(shader);
  }
}

inline void LinkShaderProgram(const GLuint program) {
  glLinkProgram(program);
  GLint status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    throw Error("Failed to link shader program").WithMessage(program);
  }
}

ValueObject ShaderProgramCreate() {
  InitLoaderOnce();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);

  std::vector<ValueObject> shaders;
  for(const auto [code, stage] : GetShaders()) {
    ValueObject shader(glCreateShader(stage), glDeleteShader);
    CompileShader(code.data(), shader.Value());
    shaders.push_back(std::move(shader));
  }
  ValueObject program(glCreateProgram(), glDeleteProgram);
  for(const ValueObject& shader : shaders) {
    glAttachShader(program.Value(), shader.Value());
  }
  LinkShaderProgram(program.Value());

  return program;
}

} // namespace

Renderer::Renderer(Window& window)
    : window_(window),
      program_(ShaderProgramCreate()),
      object_() {}

void Renderer::LoadModel(const std::string& path) {
  object_ = ObjectLoader(program_).Load(path);
}

void Renderer::RenderFrame() {
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(program_.Value());
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object_.ebo.Value());
  glBindBuffer(GL_ARRAY_BUFFER, object_.vbo.Value());
  const engine::Uniforms& uniforms = model_.GetUniforms();

  GLint uniform_location = glGetUniformLocation(program_.Value(), "ubo.model");
  glUniformMatrix4fv(uniform_location, 1, GL_FALSE, glm::value_ptr(uniforms.model[0]));
  uniform_location = glGetUniformLocation(program_.Value(), "ubo.view");
  glUniformMatrix4fv(uniform_location, 1, GL_FALSE, glm::value_ptr(uniforms.view[0]));
  uniform_location = glGetUniformLocation(program_.Value(), "ubo.proj");

  glUniformMatrix4fv(uniform_location, 1, GL_FALSE, glm::value_ptr(uniforms.proj[0]));

  size_t prev_offset = 0;

  for(const auto[index, offset] : object_.usemtl) {
    glBindTexture(GL_TEXTURE_2D, object_.textures[index].Value());
    glDrawElements(GL_TRIANGLES, offset - prev_offset, GL_UNSIGNED_INT, reinterpret_cast<void*>(prev_offset * sizeof(GLuint)));
    prev_offset = offset;
  }
}

} // namespace gl