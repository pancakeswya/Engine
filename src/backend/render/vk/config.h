#ifndef BACKEND_RENDER_VK_CONFIG_H_
#define BACKEND_RENDER_VK_CONFIG_H_

#include <vulkan/vulkan.h>
#include <stb_image.h>

#include <vector>

namespace render::vk {

struct ImageSettings {
  int stbi_format;
  VkFormat vk_format;

  VkExtent2D dummy_image_extent;
};

struct Config {
  VkApplicationInfo app_info;

  ImageSettings image_settings;

  size_t frame_count;

  std::vector<const char*> instance_extensions;
  std::vector<const char*> device_extensions;

  std::vector<VkDynamicState> dynamic_states;
  std::vector<VkPipelineStageFlags> pipeline_stages;
};

} // namespace vk

#endif // BACKEND_RENDER_VK_CONFIG_H_