#ifndef ENGINE_CAST_UTIL_H_
#define ENGINE_CAST_UTIL_H_

#include "engine/error.h"

#include <stdexcept>
#include <string>

#define NAMED_DYNAMIC_CAST(type, val) engine::cast_util::named_dynamic_cast<type, decltype(val)>(#type, val)

namespace engine::cast_util {

template <typename T1, typename T2>
static inline T1 named_dynamic_cast(const char* name, T2 val) {
  try {
    return dynamic_cast<T1>(val);
  } catch ([[maybe_unused]]const std::bad_cast& e) {
    throw Error("Failed to cast dynamic cast to " + std::string(name));
  }
}

} // namespace engine

#endif // ENGINE_CAST_UTIL_H_
