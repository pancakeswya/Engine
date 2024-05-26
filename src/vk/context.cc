#include "vk/context.h"

namespace vk {


Context::Context(glfw::Window& window)
  : instance_(),
#ifdef DEBUG
    messenger_(
      instance_.get()
    ),
#endif
    surface_(
      instance_.get(),
      window.window_
    ),
    devices_(
      instance_.get(),
      surface_.get()
    ),
    swap_chain_(
      window.window_,
      devices_.get_physical(),
      devices_.get_logical(),
      surface_.get()
    ),
    render_pass_(
      devices_.get_logical(),
      swap_chain_.get_format()
    ),
    pipeline_layout_(
      devices_.get_logical()
    ),
    pipeline_(
      devices_.get_logical(),
      pipeline_layout_.get(),
      render_pass_.get()
    ),
    cmd_pool_(
      devices_.get_logical(),
      devices_.get_physical(),
      surface_.get()
    ),
    cmd_buffer_(
      devices_.get_logical(),
      cmd_pool_.get()
    ),
    image_semaphore_(
      devices_.get_logical()
    ),
    render_semaphore_(
      devices_.get_logical()
    ),
    fence_(
      devices_.get_logical()
    ) {
    auto images = swap_chain_.get_images();
    views_.reserve(images.size());
    for(auto image : images) {
      views_.emplace_back(
        devices_.get_logical(),
        image,
        swap_chain_.get_format()
      );
    }
    framebuffers_.reserve(views_.size());
    for(auto& view : views_) {
      framebuffers_.emplace_back(
       devices_.get_logical(),
       render_pass_.get(),
       view.get(),
       swap_chain_.get_extent()
      );
    }
}

Context::~Context() {
  devices_.WaitIdle();
}

void Context::Render() {
  fence_.Wait();
  fence_.Reset();

  const uint32_t image_idx = image_semaphore_.AcquireNextImage(swap_chain_.get_swapchain());

  cmd_buffer_.Reset();
  command::Record record = cmd_buffer_.BeginRecord();
  {
    auto clear_color = VkClearValue{{0.0f, 0.0f, 0.0f, 1.0f}};
    auto render_pass_begin_info = render_pass_.BeginInfo(
      framebuffers_[image_idx].get(),
      swap_chain_.get_extent(),
      &clear_color
    );
    record.BeginRenderPass(&render_pass_begin_info);
    {
      record.BindPipeline(pipeline_.get());

      VkViewport viewport = {};
      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = static_cast<float>(swap_chain_.get_extent().width);
      viewport.height = static_cast<float>(swap_chain_.get_extent().height);
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;

      record.SetViewport(&viewport);

      VkRect2D scissor = {};
      scissor.offset = {0, 0};
      scissor.extent = swap_chain_.get_extent();

      record.SetScissor(&scissor);
      record.Draw();
    } record.EndRenderPass();
  } record.End();

  devices_.SubmitDraw(
    fence_.get(),
    cmd_buffer_.get(),
    image_semaphore_.get(),
    render_semaphore_.get()
  );
  devices_.SubmitPresentImage(
    image_idx,
    render_semaphore_.get(),
    swap_chain_.get_swapchain()
  );
}


}