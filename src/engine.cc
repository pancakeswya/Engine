#include "engine.h"
#include "base/exception.h"
#include "glfw/window.h"
#include "vk/instance.h"
#include "vk/devices.h"
#include "vk/graphics.h"
#include "vk/images.h"
#include "vk/surface.h"
#include "vk/swap_chain.h"
#include "vk/render.h"
#include "vk/shader.h"

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
    auto swap_chain = vk::SwapChain(window.Get(), devices.GetPhysical(), devices.GetLogical(), surface.Get());
    auto images = vk::Images(devices.GetLogical(), swap_chain.GetChain(), swap_chain.GetFormat());
    auto pass = vk::render::Pass(devices.GetLogical(), swap_chain.GetFormat());
    auto pipeline_layout = vk::graphics::Pipeline::Layout(devices.GetLogical());
    auto pipeline = vk::graphics::Pipeline(devices.GetLogical(), pipeline_layout.Get(), pass.Get());
    (void)pipeline.Get();
    window.Poll();
  } catch (const Exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

} // namespace engine