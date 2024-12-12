#ifndef BACKEND_RENDER_MODEL_H_
#define BACKEND_RENDER_MODEL_H_

#include "backend/render/types.h"

namespace render {

class Model {
public:
  explicit Model(Uniforms* uniforms);
  [[nodiscard]] bool ViewIsSet() const noexcept;
  void SetView(int width, int height) const noexcept;
  void Rotate(float degrees) const noexcept;
private:
  Uniforms* uniforms_;
  mutable bool view_is_set_;
};

inline Model::Model(Uniforms* uniforms)
  : uniforms_(uniforms), view_is_set_(false) {}

inline bool Model::ViewIsSet() const noexcept { return view_is_set_; }

} // namespace render

#endif // BACKEND_RENDER_MODEL_H_
