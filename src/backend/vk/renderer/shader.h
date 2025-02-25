#ifndef BACKEND_VK_RENDERER_SHADER_H_
#define BACKEND_VK_RENDERER_SHADER_H_

#include <vulkan/vulkan.h>

#include <string_view>
#include <vector>

#include "backend/vk/renderer/handle.h"

namespace vk {

struct ShaderDescription {
  VkShaderStageFlagBits stage;
  std::string_view entry_point;
};

struct ShaderInfo {
  ShaderDescription description;
  std::vector<uint32_t> spirv;
};

struct Shader {
  static std::vector<ShaderInfo> GetInfos();

  DeviceHandle<VkShaderModule> module;
  ShaderDescription description;
};

} // namespace vk

#endif //BACKEND_VK_RENDERER_SHADER_H_