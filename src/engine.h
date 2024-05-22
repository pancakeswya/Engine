#ifndef ENGINE_H_
#define ENGINE_H_

namespace engine {

constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 600;
constexpr char kTitle[] = "Vulkan";

int Run() noexcept;

} // namespace engine

#endif // ENGINE_H_