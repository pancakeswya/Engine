#ifndef BACKEND_VK_RENDERER_SHADERS_H_
#define BACKEND_VK_RENDERER_SHADERS_H_

#include <vector>
#include <vulkan/vulkan.h>
#include <string_view>

namespace vk {

struct Shader {
  VkShaderStageFlagBits stage;
  std::vector<uint32_t> spirv;
  std::string_view entry_point;
};

extern std::vector<Shader> GetShaders();

} // namespace vk


#endif //BACKEND_VK_RENDERER_SHADERS_H_