#ifndef BACKEND_WINDOW_GLFW_INSTANCE_H_
#define BACKEND_WINDOW_GLFW_INSTANCE_H_

#include <memory>

namespace window::glfw {

class Instance {
 public:
  using Handle = std::unique_ptr<Instance>;

  static Handle Init();

  ~Instance();
 private:
  Instance();
};

} // namespace window::glfw

#endif // BACKEND_WINDOW_GLFW_INSTANCE_H_