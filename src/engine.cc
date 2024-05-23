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
    auto messenger = vk::debug::Messenger(instance.Get());
    (void)messenger.Get();
#endif
    auto surface = vk::Surface(instance.Get(), window.Get());
    auto physical_device = vk::device::physical::Find(instance.Get(), surface.Get());
    auto logic_device = vk::device::Logical(physical_device, surface.Get());
    (void)logic_device.GetGraphicsQueue();
    (void)logic_device.GetPresentQueue();
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