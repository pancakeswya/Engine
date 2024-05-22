#ifndef VK_COMMON_H_
#define VK_COMMON_H_

#include <array>
#include <vector>

namespace vk::common {

constexpr std::array kValidationLayers = {
  "VK_LAYER_KHRONOS_validation"
};

namespace extensions {

std::vector<const char*> Get();

} // namespace extensionss

} // namespace vk

#endif // VK_COMMON_H_