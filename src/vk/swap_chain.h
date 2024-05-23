#ifndef VK_SWAP_CHAIN_H_
#define VK_SWAP_CHAIN_H_

#include <vector>
#include <vulkan/vulkan.h>

namespace vk {

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

} // namespace vk

#endif // VK_SWAP_CHAIN_H_