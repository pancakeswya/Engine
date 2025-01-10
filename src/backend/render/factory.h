#ifndef BACKEND_RENDER_FACTORY_H_
#define BACKEND_RENDER_FACTORY_H_

#include "backend/render/render.h"

namespace render {

class Factory {
public:
  explicit Factory(Type type) noexcept;

  Renderer::Handle CreateRenderer(window::Window& window) const;
private:
  Type type_;
};

inline Factory::Factory(const Type type) noexcept : type_(type) {}

} // namespace render

#endif // BACKEND_RENDER_FACTORY_H_