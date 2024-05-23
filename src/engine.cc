#include "engine.h"
#include "base/exception.h"
#include "glfw/window.h"
#include "vk/instance.h"
#include "vk/devices.h"
#include "vk/debug.h"
#include "vk/surface.h"
#include "vk/swap_chain.h"

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
    (void)instance;
#ifdef DEBUG
    auto messenger = vk::debug::Messenger(instance.Get());
    (void)messenger.Get();
#endif
    auto surface = vk::Surface(instance.Get(), window.Get());
    auto devices = vk::Devices(instance.Get(), surface.Get());
    (void)devices.GetLogical();
    (void)devices.GetPhysical();
    (void)devices.GetGraphicsQueue();
    (void)devices.GetPresentQueue();
    auto swap_chain = vk::SwapChain(window.Get(), devices.GetPhysical(), devices.GetLogical(), surface.Get());
    (void)swap_chain.GetChain();
    // (void)logical_device.GetPhysicalDevice();
    // (void)logical_device.GetGraphicsQueue();
    // (void)logical_device.GetPresentQueue();
    window.Poll();
  } catch (const Exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

} // namespace engine