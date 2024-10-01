#ifndef BACKEND_RENDER_VK_BACKEND_H_
#define BACKEND_RENDER_VK_BACKEND_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace vk {

class BackendImpl;

class Backend final {
public:
  explicit Backend(GLFWwindow* window);
  ~Backend();

  void Render() const;
  void LoadModel(const std::string& path);
private:
  BackendImpl* impl_;
};

} // namespace vk::backend

#endif // BACKEND_RENDER_VK_BACKEND_H_