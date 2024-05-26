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

}