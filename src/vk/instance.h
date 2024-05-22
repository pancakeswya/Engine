#ifndef VK_INSTANCE_H_
#define VK_INSTANCE_H_

#include <vulkan/vulkan.h>

namespace vk {

class Instance {
public:
  static VkInstance& Get();
private:
  Instance();
  ~Instance();

  VkInstance instance_;
};

inline VkInstance& Instance::Get() {
    static Instance i;
    return i.instance_;
}

} // namespace vk

#endif // VK_INSTANCE_H_