#ifndef VK_CONTEXT_H_
#define VK_CONTEXT_H_

#include "vk/command.h"
#include "vk/devices.h"
#include "vk/framebuffer.h"
#ifdef DEBUG
#include "vk/messenger.h"
#endif
#include "vk/instance.h"
#include "vk/render_pass.h"
#include "vk/pipeline.h"
#include "vk/sync.h"
#include "vk/surface.h"
#include "vk/swap_chain.h"
#include "glfw/window.h"

#include <vulkan/vulkan.h>

namespace vk {

class Context;

} // namespace vk

namespace render {

void Render(vk::Context& context);

}

namespace vk {

class Context {
public:
  explicit Context(glfw::Window& window);
  ~Context();

private:
  friend void render::Render(Context& context);

  Instance instance_;
#ifdef DEBUG
  Messenger messenger_;
#endif
  Surface surface_;
  Devices devices_;
  SwapChain swap_chain_;
  RenderPass render_pass_;
  std::vector<VkImage> images_;
  std::vector<ImageView> views_;
  std::vector<Framebuffer> framebuffers_;
  Pipeline::Layout pipeline_layout_;
  Pipeline pipeline_;
  command::Pool cmd_pool_;
  command::Buffers cmd_buffers_;
  sync::ImageSemaphore image_semaphore_;
  sync::Semaphore render_semaphore_;
  sync::Fence fence_;
};

} // namespace vk


#endif // VK_CONTEXT_H_