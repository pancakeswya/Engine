#ifndef BACKEND_VK_RENDERER_ERROR_H_
#define BACKEND_VK_RENDERER_ERROR_H_

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <string>

namespace vk {

struct Error final : std::runtime_error {
  using runtime_error::runtime_error;

  [[nodiscard]] Error WithCode(const VkResult result) const {
    return Error{std::string(what()) + " [Code: " + std::to_string(result) + ']'};
  }
};

} // namespace vk

#endif // BACKEND_VK_RENDERER_ERROR_H_