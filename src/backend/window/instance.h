#ifndef BACKEND_WINDOW_INSTANCE_H_
#define BACKEND_WINDOW_INSTANCE_H_

#include <memory>

namespace window {

class Instance {
public:
  using Handle = std::unique_ptr<Instance>;

  virtual ~Instance() = default;
};

} // namespace window

#endif // BACKEND_WINDOW_INSTANCE_H_
