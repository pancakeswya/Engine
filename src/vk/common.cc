#include "vk/common.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace vk::common {

namespace extensions {

std::vector<const char*> Get() {
  uint32_t ext_count = 0;
  const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&ext_count);
  std::vector extensions(glfw_extensions, glfw_extensions + ext_count);
#ifdef DEBUG
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
  return extensions;
}

} // namespace extensions

} // namespace vk