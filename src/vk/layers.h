#ifndef VK_COMMON_H_
#define VK_COMMON_H_

#include <array>
#include <vector>

namespace vk::layers {

constexpr std::array kValidation = {
    "VK_LAYER_KHRONOS_validation"
};

std::vector<const char*> Extension();
bool ValidationSupport();

} // namespace vk

#endif // VK_COMMON_H_