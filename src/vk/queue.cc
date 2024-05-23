#include "vk/queue.h"
#include "vk/devices.h"
#include "vk/swap_chain.h"

#include <optional>
#include <vector>

namespace vk::queue {

std::pair<bool, FamilyIndices> FindFamilyIndices(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
) {
  std::optional<uint32_t> graphic, present;
  uint32_t families_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &families_count, nullptr);

  std::vector<VkQueueFamilyProperties> families(families_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &families_count,
                                           families.data());

  for (size_t i = 0; i < families_count; ++i) {
    if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphic = static_cast<uint32_t>(i);
    }
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
    if (present_support) {
      present = static_cast<uint32_t>(i);
    }
    if (graphic.has_value() &&
        present.has_value() &&
        Devices::ExtensionSupport(device)) {
      auto details = SwapChain::Support(device , surface);
      if (!details.formats.empty() && !details.present_modes.empty()) {
        return {true, {graphic.value(), present.value()}};
      }
        }
  }
  return {};
}

} // namespace queue