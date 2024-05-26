#include "render/render.h"
#include "vk/context.h"

namespace render {

void Render(vk::Context& context) {
  context.fence_.Wait();
  context.fence_.Reset();

  const uint32_t image_idx = context.image_semaphore_.AcquireNextImage(
    context.swap_chain_.get_swapchain()
  );

  context.cmd_buffer_.Reset();
  vk::command::Record record = context.cmd_buffer_.BeginRecord();
  {
    auto clear_color = VkClearValue{{0.0f, 0.0f, 0.0f, 1.0f}};
    auto render_pass_begin_info = context.render_pass_.BeginInfo(
      context.framebuffers_[image_idx].get(),
      context.swap_chain_.get_extent(),
      &clear_color
    );
    record.BeginRenderPass(&render_pass_begin_info);
    {
      record.BindPipeline(context.pipeline_.get());

      VkViewport viewport = {};
      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = static_cast<float>(
        context.swap_chain_.get_extent().width
      );
      viewport.height = static_cast<float>(
        context.swap_chain_.get_extent().height
      );
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;

      record.SetViewport(&viewport);

      VkRect2D scissor = {};
      scissor.offset = {0, 0};
      scissor.extent = context.swap_chain_.get_extent();

      record.SetScissor(&scissor);
      record.Draw();
    } record.EndRenderPass();
  } record.End();

  context.devices_.SubmitDraw(
    context.fence_.get(),
    context.cmd_buffer_.get(),
    context.image_semaphore_.get(),
    context.render_semaphore_.get()
  );
  context.devices_.SubmitPresentImage(
    image_idx,
    context.render_semaphore_.get(),
    context.swap_chain_.get_swapchain()
  );
}

} // namespace render