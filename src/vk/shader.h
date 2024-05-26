#ifndef VK_SHADER_H_
#define VK_SHADER_H_

#include <string>
#include <vulkan/vulkan.h>

namespace vk::shader {

struct Stage {
  VkShaderStageFlagBits bits;
  VkShaderModule module;
  std::string_view name;
};

class Module {
public:
  Module(
    VkDevice logical_device,
    const std::string& path
  );
  ~Module();

  VkShaderModule get() noexcept;
private:
  VkDevice logical_device_;
  VkShaderModule module_;
};

inline VkShaderModule Module::get() noexcept {
  return module_;
}

} // namespace vk

#endif // VK_SHADER_H_