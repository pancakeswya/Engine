#ifndef BACKEND_VK_RENDERER_CONFIG_H_
#define BACKEND_VK_RENDERER_CONFIG_H_

#include <vulkan/vulkan.h>
#include <stb_image.h>

#include <vector>

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

  std::vector<const char*> instance_extensions;
  std::vector<const char*> device_extensions;
};

extern Config DefaultConfig();

} // namespace vk

#endif // BACKEND_VK_RENDERER_CONFIG_H_