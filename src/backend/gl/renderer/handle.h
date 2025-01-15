#ifndef BACKEND_GL_RENDERER_HANDLE_H_
#define BACKEND_GL_RENDERER_HANDLE_H_

#include <GL/glew.h>

namespace gl {

class ValueObject {
public:
  using Deleter = void(*)(GLuint);

  ValueObject() = default;
  ValueObject(const ValueObject& other) = delete;
  ValueObject(ValueObject&& other) noexcept;
  ValueObject(GLuint value, Deleter deleter);
  ~ValueObject();

  ValueObject& operator=(const ValueObject&) = delete;
  ValueObject& operator=(ValueObject&&) noexcept;

  [[nodiscard]] GLuint Value() const noexcept;
private:
  GLuint value_;
  Deleter deleter_;
};

inline GLuint ValueObject::Value() const noexcept {
  return value_;
}

class ArrayObject {
public:
  using Creator = void(*)(GLsizei, GLuint*);
  using Deleter = void(*)(GLsizei, const GLuint*);

  ArrayObject() = default;
  ArrayObject(const ArrayObject& other) = delete;
  ArrayObject(ArrayObject&& other) noexcept;
  ArrayObject(GLsizei count, Creator creator, Deleter deleter);
  ~ArrayObject();

  ArrayObject& operator=(const ArrayObject& other) = delete;
  ArrayObject& operator=(ArrayObject&& other) noexcept;

  [[nodiscard]] GLuint Value() const noexcept;
private:
  GLsizei size_;
  GLuint array_;
  Deleter deleter_;
};

inline GLuint ArrayObject::Value() const noexcept {
  return array_;
}

} // namespace gl

#endif // BACKEND_GL_RENDERER_HANDLE_H_
