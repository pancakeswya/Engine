#ifndef VK_DEBUG_H_
#define VK_DEBUG_H_

#include <array>
#include <vector>

#include <vulkan/vulkan.h>

namespace vk {

class Instance;

namespace debug {

class Messenger {
  static VkDebugUtilsMessengerCreateInfoEXT CreateInfo() noexcept;

 public:
  static inline const auto kCreateInfo = CreateInfo();

  explicit Messenger(VkInstance instance);
  ~Messenger();

  const VkDebugUtilsMessengerEXT& Get() noexcept;

 private:
  static inline PFN_vkCreateDebugUtilsMessengerEXT create_;
  static inline PFN_vkDestroyDebugUtilsMessengerEXT destroy_;

  VkInstance instance_;
  VkDebugUtilsMessengerEXT messenger_;
};

inline const VkDebugUtilsMessengerEXT& Messenger::Get() noexcept {
  return messenger_;
}

} // namespace debug

} // namespace vk

#endif // VK_DEBUG_H_