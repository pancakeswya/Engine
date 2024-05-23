#ifndef VK_INSTANCE_H_
#define VK_INSTANCE_H_

#include <vulkan/vulkan.h>

namespace vk {

class Instance {
public:
 VkInstance& Get();
  Instance();
  ~Instance();
private:
  VkInstance instance_;
};

inline VkInstance& Instance::Get() {
    return instance_;
}

} // namespace vk

#endif // VK_INSTANCE_H_