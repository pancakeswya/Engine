#include "backend/gl/renderer/renderer.h"

#include <map>
#include <memory>
#include <utility>
#include <optional>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "engine/render/data_util.h"
#include "backend/gl/renderer/error.h"
#include "obj/parser.h"

#include <glm/gtc/type_ptr.hpp>

namespace gl {

namespace {

inline constexpr auto kVertexShaderSource =
  R"(
  struct UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
  };

  attribute vec3 inPosition;
  attribute vec3 inNormal;
  attribute vec2 inTexCoord;

  varying vec2 fragTexCoord;
  varying vec3 fragNormal;

  uniform UniformBufferObject ubo;

  void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
    fragNormal = inNormal;
  }
)";

inline constexpr auto kFragmentShaderSource =
R"(
  varying vec2 fragTexCoord;
  uniform sampler2D texSampler;
  void main() {
    gl_FragColor = texture2D(texSampler, fragTexCoord);
  }
)";

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

ValueHandle ShaderProgramCreate() {
  static bool is_init = false;
  if (!is_init) {
    stbi_set_flip_vertically_on_load(true);
    if (glewInit() != GLEW_OK) {
        throw Error("Failed to gl loader");
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    is_init = true;
  }
  const ValueHandle vertex_shader(glDeleteShader, glCreateShader(GL_VERTEX_SHADER));
  CompileShader(kVertexShaderSource, vertex_shader.Value());

  const ValueHandle fragment_shader(glDeleteShader, glCreateShader(GL_FRAGMENT_SHADER));
  CompileShader(kFragmentShaderSource, fragment_shader.Value());

  ValueHandle program(glDeleteProgram, glCreateProgram());
  glAttachShader(program.Value(), vertex_shader.Value());
  glAttachShader(program.Value(), fragment_shader.Value());
  LinkShaderProgram(program.Value());

  return program;
}

PointerHandle TextureCreate(const uint8_t* data, int width, int height) {
  auto texture_deleter = [](GLuint* texture) {
    glDeleteTextures(1, texture);
  };

  PointerHandle texture(texture_deleter);

  glGenTextures(1, texture.Ptr());
  glBindTexture(GL_TEXTURE_2D, texture.Value());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  return texture;
}

PointerHandle LoadDummyTexture()  {
  constexpr int dummy_width = 16;
  constexpr int dummy_height = 16;

  std::vector<unsigned char> dummy_colors(dummy_width * dummy_height, 0xff);

  return TextureCreate(dummy_colors.data(), dummy_width, dummy_height);
}

std::optional<PointerHandle> LoadTexture(const std::string& path) {
  int image_width, image_height, image_channels;
  const std::unique_ptr<stbi_uc, void(*)(void*)> pixels(stbi_load(path.c_str(), &image_width, &image_height, &image_channels, STBI_rgb_alpha), stbi_image_free);
  if (pixels == nullptr) {
    return std::nullopt;
  }
  return TextureCreate(pixels.get(), image_width, image_height);
}

std::vector<PointerHandle> LoadTextures(const obj::Data& data) {
  std::vector<PointerHandle> textures;
  textures.reserve(data.mtl.size());

  for(const obj::NewMtl& mtl : data.mtl) {
    PointerHandle texture;
    const std::string& path = mtl.map_kd;
    if (std::optional<PointerHandle> opt_texture = LoadTexture(path); !opt_texture.has_value()) {
      texture = LoadDummyTexture();
    } else {
      texture = std::move(opt_texture.value());
    }
    textures.emplace_back(std::move(texture));
  }
  return textures;
}

} // namespace

Renderer::Renderer(Window& window)
    : window_(window),
      program_(ShaderProgramCreate()),
      object_() {}

void Renderer::LoadModel(const std::string& path) {
  obj::Data data = obj::ParseFromFile(path);

  const auto buffer_deleter = std::bind(glDeleteBuffers, 1, std::placeholders::_1);

  PointerHandle ebo(buffer_deleter);
  glGenBuffers(1, ebo.Ptr());
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.Value());
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(engine::Index) * data.indices.size(),  nullptr, GL_STATIC_DRAW);

  auto indices = static_cast<engine::Index*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));

  PointerHandle vbo(buffer_deleter);
  glGenBuffers(1, vbo.Ptr());
  glBindBuffer(GL_ARRAY_BUFFER, vbo.Value());
  glBufferData(GL_ARRAY_BUFFER, sizeof(engine::Vertex) * data.indices.size(), nullptr, GL_STATIC_DRAW);
  auto vertices = static_cast<engine::Vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

  engine::data_util::RemoveDuplicates(data, vertices, indices);

  const GLuint pos_loc = glGetAttribLocation(program_.Value(), "inPosition");
  glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE,  8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(pos_loc);

  const GLuint normal_loc = glGetAttribLocation(program_.Value(), "inNormal");
  glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE,  8 * sizeof(float), (void*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(normal_loc);

  const GLuint tex_loc = glGetAttribLocation(program_.Value(), "inTexCoord");
  glVertexAttribPointer(tex_loc, 2, GL_FLOAT, GL_FALSE,  8 * sizeof(float), (void*)(6 * sizeof(GLfloat)));
  glEnableVertexAttribArray(tex_loc);

  glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
  glUnmapBuffer(GL_ARRAY_BUFFER);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  object_.vbo = std::move(vbo);
  object_.ebo = std::move(ebo);
  object_.textures = LoadTextures(data);
  object_.usemtl = std::move(data.usemtl);
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