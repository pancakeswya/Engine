#ifndef VK_QUEUE_H_
#define VK_QUEUE_H_

#include <utility>
#include <vulkan/vulkan.h>

namespace vk::queue {

struct FamilyIndices {
  uint32_t graphic, present;
};

std::pair<bool, FamilyIndices> FindFamilyIndices(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
);

} // namespace queue

#endif // VK_QUEUE_H_