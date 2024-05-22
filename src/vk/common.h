#ifndef VK_COMMON_H_
#define VK_COMMON_H_

#include <vector>
#include <vulkan/vulkan.h>

namespace vk::common {

std::vector<const char*> RequiredExtensions();

} // namespace vk

#endif // VK_COMMON_H_