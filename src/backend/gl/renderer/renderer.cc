#include "backend/gl/renderer/renderer.h"

#include <GL/glew.h>

#include <vector>

#include "backend/gl/renderer/error.h"
#include "backend/gl/renderer/object_loader.h"
#include "backend/gl/renderer/shaders.h"

namespace gl {

namespace {

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
  if (glewInit() != GLEW_OK) {
    throw Error("Failed to gl loader");
  }
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);

  std::vector<ValueObject> shaders;
  for(const auto [code, stage] : Shader::GetShaders()) {
    ValueObject shader(glCreateShader(stage), glDeleteShader);
    CompileShader(code.data(), shader.Value());
    shaders.push_back(std::move(shader));
  }
  ValueObject program(glCreateProgram(), glDeleteProgram);
  for(const ValueObject& shader : shaders) {
    glAttachShader(program.Value(), shader.Value());
  }
  LinkShaderProgram(program.Value());
  glUseProgram(program.Value());

  return program;
}

} // namespace

Renderer::Renderer(Window& window)
    : window_(window),
      program_(ShaderProgramCreate()),
      uniform_updater_(program_.Value()),
      object_() {
  ObjectLoader::Init();
  window.SetWindowResizedCallback([](const int width, const int height) {
    glViewport(0, 0, width, height);
  });
}

void Renderer::LoadModel(const std::string& path) {
  object_ = ObjectLoader(program_).Load(path);
}

void Renderer::RenderFrame() {
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  uniform_updater_.Update(model_.GetUniforms());

  size_t prev_offset = 0;

  for(const auto[index, offset] : object_.usemtl) {
    glBindTexture(GL_TEXTURE_2D, object_.textures[index].Value());
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(offset - prev_offset), GL_UNSIGNED_INT, reinterpret_cast<void*>(prev_offset * sizeof(GLuint)));
    prev_offset = offset;
  }
  glFinish();
}

} // namespace gl