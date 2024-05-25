#include "engine.h"
#include "base/exception.h"
#include "glfw/window.h"
#include "vk/instance.h"
#include "vk/devices.h"
#include "vk/pipeline.h"
#include "vk/surface.h"
#include "vk/command.h"
#include "vk/swap_chain.h"
#include "vk/render_pass.h"
#include "vk/framebuffer.h"
#include "vk/sync.h"

#ifdef DEBUG
#include "vk/messenger.h"
#endif

#include <cstdlib>
#include <iostream>


namespace engine {

constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 600;
constexpr char kTitle[] = "Vulkan";

int Run() noexcept {
  try {
    glfw::Window window(kTitle, kWindowWidth, kWindowHeight);
    auto instance = vk::Instance();
#ifdef DEBUG
    auto messenger = vk::Messenger(instance.Get());
    (void)messenger.Get();
#endif
    auto surface = vk::Surface(instance.Get(), window.Get());
    auto devices = vk::Devices(instance.Get(), surface.Get());
    auto swap_chain = vk::SwapChain(
      window.Get(),
      devices.Physical(),
      devices.Logical(),
      surface.Get()
    );
    auto images = swap_chain.Images();
    std::vector<vk::ImageView> views;
    views.reserve(images.size());
    for(auto image : images) {
      views.emplace_back(
        devices.Logical(),
        image,
        swap_chain.Format()
      );
    }
    auto render_pass = vk::RenderPass(devices.Logical(), swap_chain.Format());
    auto pipeline_layout = vk::Pipeline::Layout(devices.Logical());
    auto pipeline = vk::Pipeline(
      devices.Logical(),
      pipeline_layout.Get(),
      render_pass.Get()
    );
    std::vector<vk::Framebuffer> framebuffers;
    framebuffers.reserve(views.size());
    for(auto& view : views) {
      framebuffers.emplace_back(
       devices.Logical(),
       render_pass.Get(),
       view.Get(),
       swap_chain.Extent()
      );
    }
    auto cmd_pool = vk::command::Pool(
      devices.Logical(),
      devices.Physical(),
      surface.Get()
    );
    auto cmd_buffer = vk::command::Buffer(
      devices.Logical(),
      cmd_pool.Get()
    );
    auto image_available_semaphore = vk::sync::ImageSemaphore(devices.Logical());
    auto render_finished_semaphore = vk::sync::Semaphore(devices.Logical());
    auto fence = vk::sync::Fence(devices.Logical());
    auto render_call = [&]{
      fence.Wait();
      fence.Reset();

      const uint32_t image_idx = image_available_semaphore.AcquireNextImage(swap_chain.Get());

      cmd_buffer.Reset();
      auto record = cmd_buffer.BeginRecord();
      {
        auto clear_color = VkClearValue{{0.0f, 0.0f, 0.0f, 1.0f}};
        auto render_pass_begin_info = render_pass.BeginInfo(
          framebuffers[image_idx].Get(),
          swap_chain.Extent(),
          &clear_color
        );
        record.BeginRenderPass(&render_pass_begin_info);
        {
          record.BindPipeline(pipeline.Get());

          VkViewport viewport = {};
          viewport.x = 0.0f;
          viewport.y = 0.0f;
          viewport.width = static_cast<float>(swap_chain.Extent().width);
          viewport.height = static_cast<float>(swap_chain.Extent().height);
          viewport.minDepth = 0.0f;
          viewport.maxDepth = 1.0f;

          record.SetViewport(&viewport);

          VkRect2D scissor = {};
          scissor.offset = {0, 0};
          scissor.extent = swap_chain.Extent();

          record.SetScissor(&scissor);
          record.Draw();
        } record.EndRenderPass();
      } record.End();

      devices.SubmitDraw(
        fence.Get(),
        cmd_buffer.Get(),
        image_available_semaphore.Get(),
        render_finished_semaphore.Get()
      );

      devices.SubmitPresentImage(
        image_idx,
        render_finished_semaphore.Get(),
        swap_chain.Get()
      );
    };

    window.Poll(render_call);
    devices.WaitIdle();
  } catch (const Exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

} // namespace engine