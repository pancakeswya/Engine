#ifndef ENGINE_WINDOW_INSTANCE_H_
#define ENGINE_WINDOW_INSTANCE_H_

#include <memory>

namespace engine {

class Instance {
public:
  using Handle = std::unique_ptr<Instance, void(*)(Instance*)>;

  virtual ~Instance() = default;
};

} // namespace engine

#endif // ENGINE_WINDOW_INSTANCE_H_
