#include "vk/shader.h"
#include "vk/exception.h"
#include "base/io.h"

namespace vk::shader {

Module::Module(
  VkDevice logical_device,
  const std::string& path
) : logical_device_(logical_device), module_() {
  std::vector<char> code = io::ReadFileBytes(path);
  VkShaderModuleCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = code.size();
  create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

  if (vkCreateShaderModule(logical_device, &create_info, nullptr, &module_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to create shader module!");
  }
}

Module::~Module() {
  vkDestroyShaderModule(logical_device_, module_, nullptr);
}

} // namespace vk::shader