#ifndef VK_DEBUG_H_
#define VK_DEBUG_H_

#include <vulkan/vulkan.h>

namespace vk::debug {

class Messenger {
 public:
  class CreateInfo {
   public:
    static const VkDebugUtilsMessengerCreateInfoEXT& Get();
   private:
    CreateInfo();
    VkDebugUtilsMessengerCreateInfoEXT info_;
  };

  explicit Messenger();
  ~Messenger();

  [[nodiscard]] const VkDebugUtilsMessengerEXT& Get() const noexcept;
 private:
  static inline PFN_vkCreateDebugUtilsMessengerEXT create_;
  static inline PFN_vkDestroyDebugUtilsMessengerEXT destroy_;

  VkDebugUtilsMessengerEXT messenger_;
};

inline const VkDebugUtilsMessengerEXT& Messenger::Get() const noexcept {
 return messenger_;
}

inline const VkDebugUtilsMessengerCreateInfoEXT& Messenger::CreateInfo::Get() {
  static CreateInfo i;
  return i.info_;
}

} // namespace vk::debug

#endif // VK_DEBUG_H_