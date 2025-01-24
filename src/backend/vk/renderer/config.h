#ifndef BACKEND_VK_RENDERER_CONFIG_H_
#define BACKEND_VK_RENDERER_CONFIG_H_

#include <vector>

#include <vulkan/vulkan.h>

namespace vk {

struct ImageSettings {
  int stbi_format;
  VkFormat vk_format;

  VkExtent2D dummy_image_extent;
};

struct Config {
  VkApplicationInfo app_info;

  ImageSettings image_settings;

  size_t frame_count;
};

extern Config DefaultConfig();

} // namespace vk

#endif // BACKEND_VK_RENDERER_CONFIG_H_