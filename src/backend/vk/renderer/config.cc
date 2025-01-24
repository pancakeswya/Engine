#include "backend/vk/renderer/config.h"

#include <stb_image.h>

namespace vk {

Config DefaultConfig() {
  Config config = {};

  config.app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  config.app_info.pApplicationName = "VulkanFun";
  config.app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  config.app_info.pEngineName = "Simple Engine";
  config.app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  config.app_info.apiVersion = VK_API_VERSION_1_0;

  config.image_settings.stbi_format = STBI_rgb_alpha;
  config.image_settings.vk_format = VK_FORMAT_R8G8B8A8_SRGB;
  config.image_settings.dummy_image_extent = VkExtent2D{16,16};

  config.frame_count = 2;

  return config;
}

} // namespace vk