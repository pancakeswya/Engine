#ifndef BACKEND_WINDOW_PROVIDER_H_
#define BACKEND_WINDOW_PROVIDER_H_

#include "backend/window/instance.h"
#include "backend/window/window.h"

namespace window {

enum class BackendType {
  kGlfw,
  kSdl
};

class Provider {
 public:
  Provider(BackendType type, Size size, const std::string& title);
  ~Provider() = default;

  [[nodiscard]] IWindow& Provide() const noexcept;
 private:
  template <typename InstanceType, typename WindowType>
  void Provide(Size size, const std::string& title);

  IInstance::Handle instance_;
  IWindow::Handle window_;
};

inline IWindow& Provider::Provide() const noexcept {
  return *window_;
}

} // namespace window

#endif // BACKEND_WINDOW_PROVIDER_H_