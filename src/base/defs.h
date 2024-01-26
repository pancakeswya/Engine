#ifndef ENGINE_SRC_BASE_DEFS_H_
#define ENGINE_SRC_BASE_DEFS_H_

#include <string_view>

namespace engine::defs {

constexpr int kWindowWidth = 640;
constexpr int kWindowHeight = 480;
constexpr std::string_view kWindowTitle = "Engine";

} // namespace engine::defs

#endif // ENGINE_SRC_BASE_DEFS_H_