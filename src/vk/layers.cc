#include "vk/layers.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstring>

namespace vk::layers {

std::vector<const char*> Extension() {
  uint32_t ext_count = 0;
  const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&ext_count);
  std::vector extensions(glfw_extensions, glfw_extensions + ext_count);
#ifdef DEBUG
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
  return extensions;
}

bool ValidationSupport() {
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
  for (const char* validation_layer : kValidation) {
    bool layer_found = false;
    for (const auto& layer_properties : available_layers) {
      if (std::strcmp(validation_layer, layer_properties.layerName) == 0) {
        layer_found = true;
        break;
      }
    }
    if (!layer_found) {
      return false;
    }
  }
  return true;
}

} // namespace vk