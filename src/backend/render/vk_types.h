#ifndef BACKEND_RENDER_VK_TYPES_H_
#define BACKEND_RENDER_VK_TYPES_H_

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <functional>
#include <memory>
#include <stdexcept>
#include <vector>
#include <type_traits>

namespace vk {

template<typename HandleType>
using HandleWrapper = std::unique_ptr<std::remove_pointer_t<HandleType>, std::function<void(HandleType)>>;

struct Error final : std::runtime_error {
  using runtime_error::runtime_error;

  [[nodiscard]] Error WithCode(const VkResult result) const {
    return Error{std::string(what()) + " [Code: " + std::to_string(result) + ']'};
  }
};

struct QueueFamilyIndices {
  uint32_t graphic, present;
};

struct PhysicalDeviceSurfaceDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

struct SwapchainDetails {
  VkExtent2D extent;
  VkFormat format;
};

struct ShaderStage {
  VkShaderStageFlagBits bits;
  HandleWrapper<VkShaderModule> module;

  std::string_view name;
};

struct Vertex {
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 tex_coord;

  static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
  static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
};

struct Index {
  using type = uint16_t;

  static constexpr VkIndexType type_enum = VK_INDEX_TYPE_UINT16;
};

struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

} // namespace vk

#endif // BACKEND_RENDER_VK_TYPES_H_