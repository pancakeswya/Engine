#ifndef VK_DEBUG_H_
#define VK_DEBUG_H_

#include <vector>

#include "vulkan/vulkan.h"

namespace vk::debug {

class Messenger {
 public:
  class CreateInfo {
   public:
    static VkDebugUtilsMessengerCreateInfoEXT Get();
   private:
    CreateInfo();
    VkDebugUtilsMessengerCreateInfoEXT info_;
  };

  Messenger(VkInstance instance);
  ~Messenger();
 private:
  static inline PFN_vkCreateDebugUtilsMessengerEXT create_;

  static inline PFN_vkDestroyDebugUtilsMessengerEXT destroy_;

  VkInstance instance_;
  VkDebugUtilsMessengerEXT messenger_;
};

inline VkDebugUtilsMessengerCreateInfoEXT Messenger::CreateInfo::Get() {
  static Messenger::CreateInfo i;
  return i.info_;
}

} // namespace vk::debug

#endif // VK_DEBUG_H_