#include "engine/config.h"

#include <sstream>

namespace engine {

namespace {

std::string GetRendererDllPath(const RendererType::Name renderer_name) {
  std::stringstream ss;
  ss << "lib" << renderer_name << "_renderer";
  return ss.str();
}

std::string GetWindowDllPath(const RendererType::Name renderer_name, const WindowType::Name window_name) {
  std::stringstream ss;
  ss << "lib" << window_name << '_' << renderer_name << "_window";
  return ss.str();
}

std::string GetTitle(const RendererType::Name renderer_name, const WindowType::Name window_name) {
  std::stringstream ss;
  ss << window_name << " " << renderer_name << " engine";

  return ss.str();
}

} // namespace

Config::Config(RendererType::Name renderer_type, WindowType::Name window_type)
  : window_plugin_path(GetWindowDllPath(renderer_type, window_type)),
    renderer_plugin_path(GetRendererDllPath(renderer_type)),
    title(GetTitle(renderer_type, window_type)) {}

} // namespace engine