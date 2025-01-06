#ifndef BACKEND_WINDOW_INSTANCE_H_
#define BACKEND_WINDOW_INSTANCE_H_

#include <memory>

namespace window {

class IInstance {
public:
  using Handle = std::unique_ptr<IInstance>;

  virtual ~IInstance() = default;
};

} // namespace window

#endif // BACKEND_WINDOW_INSTANCE_H_
