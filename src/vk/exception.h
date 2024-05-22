#ifndef VK_EXCEPTION_H_
#define VK_EXCEPTION_H_

#include "base/exception.h"

#include <string>

namespace vk {

class Exception : public engine::Exception {
 public:
  explicit Exception(const std::string& message)
    : engine::Exception("Vulkan error: " + message) {}
  ~Exception() override = default;
};

} // namespace engine::vk

#endif // VK_EXCEPTION_H_