#ifndef ENGINE_SRC_VIEW_APPLICATION_H_
#define ENGINE_SRC_VIEW_APPLICATION_H_

namespace engine {

class Application {
 public:
  Application() noexcept;
  ~Application();
  [[nodiscard]] bool IsInitialized() const noexcept;
 private:
  bool is_init_;
};

} // namespace engine

#endif // ENGINE_SRC_VIEW_APPLICATION_H_