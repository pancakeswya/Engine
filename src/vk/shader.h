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

  VkShaderModule Get() noexcept;
private:
  VkDevice logical_device_;
  VkShaderModule module_;
};

inline VkShaderModule Module::Get() noexcept {
  return module_;
}

} // namespace vk

#endif // VK_SHADER_H_