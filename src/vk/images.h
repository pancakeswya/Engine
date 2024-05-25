#ifndef VK_IMAGES_H_
#define VK_IMAGES_H_

#include <vector>
#include <vulkan/vulkan.h>

namespace vk {

class Images {
public:
  Images(
    VkDevice logical_device,
    VkSwapchainKHR swapchain,
    VkFormat format
  );
  ~Images();

  std::vector<VkImage>& GetImages() noexcept;
  std::vector<VkImageView>& GetViews() noexcept;
private:
  VkDevice logical_device_;
  std::vector<VkImage> images_;
  std::vector<VkImageView> views_;
};

inline std::vector<VkImage>& Images::GetImages() noexcept {
  return images_;
}

inline std::vector<VkImageView>& Images::GetViews() noexcept {
  return views_;
}

} // namespace vk

#endif // VK_IMAGES_H_