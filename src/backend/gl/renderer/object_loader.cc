#include "backend/gl/renderer/object_loader.h"

#include <optional>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "backend/gl/renderer/error.h"
#include "engine/render/types.h"
#include "engine/render/data_util.h"
#include "obj/parser.h"

namespace gl {

namespace {

ArrayObject TextureCreate(const uint8_t* data, const int width, const int height) {
  ArrayObject texture(1, glGenTextures, glDeleteTextures);
  glBindTexture(GL_TEXTURE_2D, texture.Value());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  return texture;
}

ArrayObject LoadDummyTexture()  {
  constexpr int dummy_width = 16;
  constexpr int dummy_height = 16;

  const std::vector<unsigned char> dummy_colors(dummy_width * dummy_height, 0xff);

  return TextureCreate(dummy_colors.data(), dummy_width, dummy_height);
}

ArrayObject LoadTexture(const std::string& path) {
  int image_width, image_height, image_channels;
  const std::unique_ptr<stbi_uc, void(*)(void*)> pixels(stbi_load(path.c_str(), &image_width, &image_height, &image_channels, STBI_rgb_alpha), stbi_image_free);
  if (pixels == nullptr) {
    return LoadDummyTexture();
  }
  return TextureCreate(pixels.get(), image_width, image_height);
}

std::vector<ArrayObject> LoadTextures(const obj::Data& data) {
  std::vector<ArrayObject> textures;
  textures.reserve(data.mtl.size());

  for(const obj::NewMtl& mtl : data.mtl) {
    const std::string& path = mtl.map_kd;
    ArrayObject texture = LoadTexture(path);
    textures.emplace_back(std::move(texture));
  }
  return textures;
}

} // namespace

Object ObjectLoader::Load(const std::string& path) const {
  stbi_set_flip_vertically_on_load(true);

  obj::Data data = obj::ParseFromFile(path);

  ArrayObject ebo(1, glGenBuffers, glDeleteBuffers);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.Value());
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(engine::Index) * data.indices.size()),  nullptr, GL_STATIC_DRAW);

  auto indices = static_cast<engine::Index*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
  if (indices == nullptr) {
    throw Error("Failed to map indices");
  }
  ArrayObject vbo(1, glGenBuffers, glDeleteBuffers);
  glBindBuffer(GL_ARRAY_BUFFER, vbo.Value());
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(engine::Vertex) * data.indices.size()), nullptr, GL_STATIC_DRAW);
  auto vertices = static_cast<engine::Vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
  if (vertices == nullptr) {
    throw Error("Failed to map vertices");
  }
  engine::data_util::RemoveDuplicates(data, vertices, indices);

  const GLuint pos_loc = glGetAttribLocation(program_.Value(), "inPosition");
  glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE,  8 * sizeof(float), nullptr);
  glEnableVertexAttribArray(pos_loc);

  const GLuint normal_loc = glGetAttribLocation(program_.Value(), "inNormal");
  glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE,  8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(normal_loc);

  const GLuint tex_loc = glGetAttribLocation(program_.Value(), "inTexCoord");
  glVertexAttribPointer(tex_loc, 2, GL_FLOAT, GL_FALSE,  8 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(GLfloat)));
  glEnableVertexAttribArray(tex_loc);

  glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
  glUnmapBuffer(GL_ARRAY_BUFFER);

  Object object = {};

  object.vbo = std::move(vbo);
  object.ebo = std::move(ebo);
  object.textures = LoadTextures(data);
  object.usemtl = std::move(data.usemtl);

  return object;
}


} // namespace gl