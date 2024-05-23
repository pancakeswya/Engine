#include "engine.h"
#include "base/exception.h"
#include "glfw/window.h"
#include "vk/instance.h"
#include "vk/device.h"
#include "vk/debug.h"
#include "vk/surface.h"

#include <cstdlib>
#include <iostream>

namespace engine {

constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 600;
constexpr char kTitle[] = "Vulkan";

int Run() noexcept {
  try {
    const glfw::Window window(kTitle, kWindowWidth, kWindowHeight);
    auto instance = vk::Instance();
    (void)instance;
#ifdef DEBUG
    auto messenger = vk::debug::Messenger(instance);
    (void)messenger.Get();
#endif
    auto surface = vk::Surface(instance, window);
    auto logical_device = vk::Device(instance, surface);
    (void)logical_device.GetLogicalDevice();
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