#ifndef BACKEND_VK_RENDERER_SHADER_H_
#define BACKEND_VK_RENDERER_SHADER_H_

#include <vector>
#include <string_view>

#include <vulkan/vulkan.h>

#include "backend/vk/renderer/dispatchable.h"

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

  DeviceDispatchable<VkShaderModule> module;
  ShaderDescription description;
};

} // namespace vk

#endif //BACKEND_VK_RENDERER_SHADER_H_