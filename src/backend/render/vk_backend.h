#ifndef VK_BACKEND_H_
#define VK_BACKEND_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vk {

class BackendImpl;

class Backend final {
public:
  explicit Backend(GLFWwindow* window);
  ~Backend();

  void Render() const;
  void LoadModel();
private:
  BackendImpl* impl_;
};

} // namespace vk::backend

#endif // VK_BACKEND_H_