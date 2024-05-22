#include "engine.h"
#include "base/exception.h"
#include "glfw/window.h"
#include "vk/instance.h"
#include "vk/device.h"

#ifdef DEBUG
#include "vk/debug.h"
#endif

#include <cstdlib>
#include <iostream>

namespace engine {

constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 600;
constexpr char kTitle[] = "Vulkan";

int Run() noexcept {
  try {
    const glfw::Window window(kTitle, kWindowWidth, kWindowHeight);
    auto vk_instance = vk::Instance::Get();
#ifdef DEBUG
    auto vk_messenger = vk::debug::Messenger(vk_instance);
    (void)vk_messenger.Get();
#endif
    auto vk_physical_device = vk::device::PhysicalFind(vk_instance);
    (void)vk_physical_device;
    auto logical_device = vk::device::Logical(vk_instance, vk_physical_device);
    auto vk_device = logical_device.GetDevice();
    (void)vk_device;
    auto vk_g_q = logical_device.GetQueue();
    (void)vk_g_q;
    window.Poll();
  } catch (const Exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

} // namespace engine