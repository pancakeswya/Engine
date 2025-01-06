#ifndef BACKEND_RENDER_MODEL_H_
#define BACKEND_RENDER_MODEL_H_

#include "backend/render/types.h"

#include <functional>

namespace render {

class Model {
public:
  class Action {
  public:
    template<typename T, typename... Args>
    explicit Action(T&& fn, Args&&... args)
      : fn_(std::bind(fn, std::placeholders::_1, std::forward<Args>(args)...)) {}

    void invoke(Model& model) const { fn_(model); }
  private:
    std::function<void(Model&)> fn_;
  };

  explicit Model(Uniforms* uniforms);
  void SetView(int width, int height) const noexcept;
  void Rotate(float degrees) const noexcept;
private:
  Uniforms* uniforms_;
};

inline Model::Model(Uniforms* uniforms) : uniforms_(uniforms) {}

} // namespace render

#endif // BACKEND_RENDER_MODEL_H_
