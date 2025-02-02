#ifndef ENGINE_CONFIG_H_
#define ENGINE_CONFIG_H_

#include <string>

namespace engine {

struct RendererType {
  using Name = std::string_view;

  static constexpr Name kVk = "vk";
  static constexpr Name kGl = "gl";
};

struct WindowType {
  using Name = std::string_view;

  static constexpr Name kGlfw = "glfw";
  static constexpr Name kSdl = "sdl";
};

struct Config {
  Config(RendererType::Name renderer_type, WindowType::Name window_type);

  std::string window_plugin_path;
  std::string renderer_plugin_path;

  std::string title;
};


} // namespace engine

#endif //ENGINE_CONFIG_H_
