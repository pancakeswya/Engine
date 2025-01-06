#include "backend/window/sdl/window.h"

namespace window::sdl {

namespace {

SDL_Window* CreateWindow(const Size size, const std::string& title) {
  SDL_Window* window = SDL_CreateWindow(title.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, size.width, size.height,  SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
  if (window == nullptr) {
    throw Error("Failed to create window");
  }
  return window;
}

} // namespace

SurfaceFactory::SurfaceFactory(SDL_Window* window) noexcept : window_(window) {}

VkSurfaceKHR SurfaceFactory::CreateSurface(VkInstance instance, [[maybe_unused]]const VkAllocationCallbacks *allocator) const {
  VkSurfaceKHR surface = VK_NULL_HANDLE;

  if (!SDL_Vulkan_CreateSurface(window_, instance, &surface)) {
    throw Error("failed to create window surface").WithMessage();
  }
  return surface;
}

Window::Window(const Size size, const std::string& title)
  : opaque_(), should_close_(false), window_(CreateWindow(size, title)), surface_factory_(window_) {}

void Window::HandleEvents() const noexcept {
  SDL_Event event;

  bool window_resized = false;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
          should_close_ = true;
          break;
      case SDL_WINDOWEVENT:
        if (event.window.windowID != SDL_GetWindowID(window_)) {
          break;
        }
        switch (event.window.event) {
          case SDL_WINDOWEVENT_RESIZED:
          case SDL_WINDOWEVENT_SIZE_CHANGED:
            window_resized = true;
            break;
          default:
            break;
        }
      default:
        break;
    }
  }
  if (window_resized) {
    opaque_.resize_callback(opaque_.user_ptr, {event.window.data1, event.window.data2});
  }
}

bool Window::ShouldClose() const noexcept { return should_close_; }

void Window::WaitUntilResized() const noexcept {
  int width = 0, height = 0;
  SDL_Vulkan_GetDrawableSize(window_, &width, &height);
  while (width == 0 || height == 0) {
    SDL_Vulkan_GetDrawableSize(window_, &width, &height);
    while (SDL_WaitEvent(nullptr))
      ;
  }
}

void Window::SetWindowUserPointer(void* user_ptr) noexcept {
  opaque_.user_ptr = user_ptr;
}

void Window::SetWindowResizedCallback(ResizeCallback resize_callback) noexcept {
  opaque_.resize_callback = resize_callback;
}

} // namespace window::sdl
