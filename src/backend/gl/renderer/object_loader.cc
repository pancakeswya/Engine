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

ArrayHandle TextureCreate(const uint8_t* data, const int width, const int height) {
  const auto texture_creator = []{
    GLuint texture_handle;
    glGenTextures(1, &texture_handle);
    return texture_handle;
  };
  const auto texture_deleter = [](GLuint* texture) {
    glDeleteTextures(1, texture);
  };
  ArrayHandle texture(texture_creator(), texture_deleter);
  glBindTexture(GL_TEXTURE_2D, texture.GetValue());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  return texture;
}

ArrayHandle LoadDummyTexture()  {
  constexpr int dummy_width = 16;
  constexpr int dummy_height = 16;

  std::vector<unsigned char> dummy_colors(dummy_width * dummy_height, 0xff);

  return TextureCreate(dummy_colors.data(), dummy_width, dummy_height);
}

std::optional<ArrayHandle> LoadTexture(const std::string& path) {
  int image_width, image_height, image_channels;
  const std::unique_ptr<stbi_uc, void(*)(void*)> pixels(stbi_load(path.c_str(), &image_width, &image_height, &image_channels, STBI_rgb_alpha), stbi_image_free);
  if (pixels == nullptr) {
    return std::nullopt;
  }
  return TextureCreate(pixels.get(), image_width, image_height);
}

std::vector<ArrayHandle> LoadTextures(const obj::Data& data) {
  std::vector<ArrayHandle> textures;
  textures.reserve(data.mtl.size());

  for(const obj::NewMtl& mtl : data.mtl) {
    ArrayHandle texture;
    const std::string& path = mtl.map_kd;
    if (std::optional<ArrayHandle> opt_texture = LoadTexture(path); !opt_texture.has_value()) {
      texture = LoadDummyTexture();
    } else {
      texture = std::move(opt_texture.value());
    }
    textures.emplace_back(std::move(texture));
  }
  return textures;
}

} // namespace

Object ObjectLoader::Load(const std::string& path) {
  stbi_set_flip_vertically_on_load(true);

  obj::Data data = obj::ParseFromFile(path);

  const auto buffer_creator = []() {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    return buffer;
  };
  const auto buffer_deleter = [](GLuint* buffers){
    glDeleteBuffers(1, buffers);
  };

  ArrayHandle ebo(buffer_creator(), buffer_deleter);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.GetValue());
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(engine::Index) * data.indices.size(),  nullptr, GL_STATIC_DRAW);

  auto indices = static_cast<engine::Index*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
  if (indices == nullptr) {
    throw Error("Failed to map indices");
  }
  ArrayHandle vbo(buffer_creator(), buffer_deleter);
  glBindBuffer(GL_ARRAY_BUFFER, vbo.GetValue());
  glBufferData(GL_ARRAY_BUFFER, sizeof(engine::Vertex) * data.indices.size(), nullptr, GL_STATIC_DRAW);
  auto vertices = static_cast<engine::Vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
  if (vertices == nullptr) {
    throw Error("Failed to map vertices");
  }
  engine::data_util::RemoveDuplicates(data, vertices, indices);

  const GLuint pos_loc = glGetAttribLocation(program_.GetValue(), "inPosition");
  glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE,  8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(pos_loc);

  const GLuint normal_loc = glGetAttribLocation(program_.GetValue(), "inNormal");
  glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE,  8 * sizeof(float), (void*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(normal_loc);

  const GLuint tex_loc = glGetAttribLocation(program_.GetValue(), "inTexCoord");
  glVertexAttribPointer(tex_loc, 2, GL_FLOAT, GL_FALSE,  8 * sizeof(float), (void*)(6 * sizeof(GLfloat)));
  glEnableVertexAttribArray(tex_loc);

  glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
  glUnmapBuffer(GL_ARRAY_BUFFER);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  Object object = {};

  object.vbo = std::move(vbo);
  object.ebo = std::move(ebo);
  object.textures = LoadTextures(data);
  object.usemtl = std::move(data.usemtl);

  return object;
}


} // namespace gl