#include <utility>

#include "backend/gl/renderer/handle_object.h"

namespace gl {

namespace {

GLuint CreateArrayObject(const GLsizei size, const ArrayObject::Creator creator) {
  GLuint array_object = 0;
  creator(size, &array_object);
  return array_object;
}

} // namespace

ValueObject::ValueObject(const GLuint value, const Deleter deleter) : value_(value), deleter_(deleter) {}

ValueObject::~ValueObject() {
  if (deleter_ != nullptr) {
    deleter_(value_);
    deleter_ = nullptr;
  }
}

ValueObject::ValueObject(ValueObject&& other) noexcept : value_(other.value_), deleter_(other.deleter_) {
  other.value_ = 0;
  other.deleter_ = nullptr;
}

ValueObject& ValueObject::operator=(ValueObject&& other) noexcept {
  if (this != &other) {
    value_ = std::exchange(other.value_, 0);
    deleter_ = std::exchange(other.deleter_, nullptr);
  }
  return *this;
}

ArrayObject::ArrayObject(const GLsizei size, const Creator creator, const Deleter deleter)
  : size_(size),  array_(CreateArrayObject(size, creator)), deleter_(deleter) {}

ArrayObject::ArrayObject(ArrayObject&& other) noexcept
  : size_(other.size_), array_(other.array_), deleter_(other.deleter_) {
  other.size_ = 0;
  other.array_ = 0;
  other.deleter_ = nullptr;
}

ArrayObject::~ArrayObject() {
  if (deleter_ != nullptr) {
    deleter_(size_, &array_);
    deleter_ = nullptr;
  }
}

ArrayObject& ArrayObject::operator=(ArrayObject&& other) noexcept {
  if (this != &other) {
    size_ = std::exchange(other.size_, 0);
    array_ = std::exchange(other.array_, 0);
    deleter_ = std::exchange(other.deleter_, nullptr);
  }
  return *this;
}

} // namespace gl
