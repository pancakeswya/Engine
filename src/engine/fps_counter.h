#ifndef ENGINE_FPS_COUNTER_H_
#define ENGINE_FPS_COUNTER_H_

namespace engine {

class FpsCounter {
public:
  FpsCounter() noexcept;
  ~FpsCounter() = default;

  double Count() noexcept;
private:
  double fps_;

  double last_update_time_;
  int frames_since_last_update_;
};

inline FpsCounter::FpsCounter() noexcept
    : fps_(0), last_update_time_(0), frames_since_last_update_(0) {}

} // namespace engine

#endif // ENGINE_FPS_COUNTER_H_
