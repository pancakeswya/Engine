#ifndef BACKEND_RENDER_VK_SHADERS_H_
#define BACKEND_RENDER_VK_SHADERS_H_

#include <vector>
#include <vulkan/vulkan.h>
#include <string_view>

namespace render::vk {

struct Shader {
  VkShaderStageFlagBits stage;
  std::vector<uint32_t> spirv;
  std::string_view entry_point;
};

extern std::vector<Shader> GetShaders();

} // namespace vk


#endif //BACKEND_RENDER_VK_SHADERS_H_