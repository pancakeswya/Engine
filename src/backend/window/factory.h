#ifndef BACKEND_WINDOW_PROVIDER_H_
#define BACKEND_WINDOW_PROVIDER_H_

#include "backend/window/instance.h"
#include "backend/window/window.h"
#include "backend/render/types.h"

#include <utility>

namespace window {

class Factory {
public:
  virtual ~Factory() = default;
  [[nodiscard]] virtual Instance::Handle CreateInstance() const = 0;
  [[nodiscard]] virtual Window::Handle CreateWindow(Size size, const std::string& title) const = 0;
};

namespace vk {

class Factory final : public window::Factory {
public:
  explicit Factory(Type type) noexcept;

  ~Factory() override = default;

  [[nodiscard]] Instance::Handle CreateInstance() const override;

  [[nodiscard]] Window::Handle CreateWindow(Size size, const std::string& title) const override;
private:
  Type type_;
};

inline Factory::Factory(const Type type) noexcept : type_(type) {}

} // namespace vk

namespace gl {

class Factory final : public window::Factory {
public:
  explicit Factory(Type type) noexcept;
  ~Factory() override = default;

  [[nodiscard]] Instance::Handle CreateInstance() const override;
  [[nodiscard]] Window::Handle CreateWindow(Size size, const std::string& title) const override;
private:
  Type type_;
};

inline Factory::Factory(const Type type) noexcept : type_(type) {}

} // namespace gl

} // namespace window

#endif // BACKEND_WINDOW_PROVIDER_H_