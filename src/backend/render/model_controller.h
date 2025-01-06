#ifndef BACKEND_RENDER_MODEL_CONTROLLER_H_
#define BACKEND_RENDER_MODEL_CONTROLLER_H_

#include "backend/render/model.h"

#include <queue>

namespace render {

class ModelController {
public:
  virtual ~ModelController() = default;
  virtual void SetAction(Model::Action) = 0;
  virtual void ExecuteAction() = 0;
  [[nodiscard]] virtual bool ActionsDone() const noexcept = 0;
};

class BufferedModelController final : public ModelController {
public:
  BufferedModelController() : curr_frame_(nullptr) {}

  explicit BufferedModelController(const std::vector<Model>& models, size_t* curr_frame)
    : curr_frame_(curr_frame), models_(models) {}

  ~BufferedModelController() override = default;

  void SetAction(const Model::Action action) override {
    const ActionInfo action_info = {action, *curr_frame_};
    action_infos_.push(action_info);
  }

  void ExecuteAction() override {
    auto& [action, frame_index] = action_infos_.front();
    action.invoke(models_[*curr_frame_]);
    if (++frame_index == models_.size()) {
      action_infos_.pop();
    }
  }

  [[nodiscard]] bool ActionsDone() const noexcept override {
    return action_infos_.empty();
  }
private:
  struct ActionInfo {
    Model::Action action;
    size_t frame_index;
  };

  size_t* curr_frame_;
  std::vector<Model> models_;

  std::queue<ActionInfo> action_infos_;
};

} // namespace render

#endif // BACKEND_RENDER_MODEL_CONTROLLER_H_