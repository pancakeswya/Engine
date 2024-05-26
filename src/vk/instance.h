#ifndef VK_INSTANCE_H_
#define VK_INSTANCE_H_

#include <vulkan/vulkan.h>

namespace vk {

class Instance {
public:
  Instance();
  ~Instance();

  VkInstance& get() noexcept;
private:
  VkInstance instance_;
};

inline VkInstance& Instance::get() noexcept {
    return instance_;
}

} // namespace vk

#endif // VK_INSTANCE_H_