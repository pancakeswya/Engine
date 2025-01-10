#include "backend/render/gl/error.h"

namespace render::gl {

namespace {

std::string GetGLMessage(const GLuint who) {
  GLint length;
  glGetShaderiv(who, GL_INFO_LOG_LENGTH, &length);

  std::string message(length, '\0');
  glGetShaderInfoLog(who, length, &length, message.data());

  return message;
}

} // namespace

Error Error::WithMessage(const GLuint who) const {
  return Error{std::string(what()) + " [Message: " + GetGLMessage(who) + ']'};
}


} // namespace render::gl