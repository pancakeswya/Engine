#ifndef ENGINE_DLL_LOADER_H_
#define ENGINE_DLL_LOADER_H_

#include "engine/error.h"

#include <string>
#include <filesystem>

#ifdef __unix__
#include <dlfcn.h>
#   ifdef __APPLE__
#       define DLL_EXT ".dylib"
#   else
#       define DLL_EXT ".so"
#   endif
#   define DL_HANDLE_TYPE void*
#   define DL_OPEN(path) dlopen(((path) + DLL_EXT).c_str(), RTLD_GLOBAL)
#   define DL_CLOSE dlclose
#   define DL_SYM dlsym
#elif defined(_WIN32)
#include <windows.h>
#   define DLL_EXT ".dll"
#   define DL_HANDLE_TYPE HINSTANCE
#   define DL_OPEN(path) LoadLibrary(((path) + DLL_EXT).c_str())
#   define DL_CLOSE FreeLibrary
#   define DL_SYM GetProcAddress
#else
#   error "not supported system"
#endif

namespace engine {

class DllLoader {
public:
  DllLoader();
  explicit DllLoader(const std::string& path);
  DllLoader(const DllLoader& other) = delete;
  DllLoader(DllLoader&& other) noexcept;
  virtual ~DllLoader();

  DllLoader& operator=(DllLoader&& other) noexcept;
  DllLoader& operator=(const DllLoader& other) = delete;

  template <typename T>
  T Load(const std::string& sym_name) const;
protected:
  DL_HANDLE_TYPE handle_;
};

inline DllLoader::DllLoader() : handle_(nullptr) {}

inline DllLoader::DllLoader(DllLoader&& other) noexcept : handle_(other.handle_) {
  other.handle_ = nullptr;
}

inline DllLoader& DllLoader::operator=(DllLoader&& other) noexcept {
  if (this != &other) {
    handle_ = other.handle_;
    other.handle_ = nullptr;
  }
  return *this;
}

inline DllLoader::DllLoader(const std::string& path) {
    if (handle_ = DL_OPEN(path); handle_ == nullptr) {
        throw Error("Failed to load library: " + path);
    }
}

inline DllLoader::~DllLoader() {
  if (handle_ != nullptr) {
    DL_CLOSE(handle_);
    handle_ = nullptr;
  }
}

template <typename T>
inline T DllLoader::Load(const std::string& sym_name) const {
  auto proc = DL_SYM(handle_, sym_name.c_str());
  if (proc == nullptr) {
    throw Error("Failed to load symbol '" + sym_name + "'");
  }
  return reinterpret_cast<T>(proc);
}

} // namespace engine

#undef DLL_EXT
#undef DL_HANDLE_TYPE
#undef DL_OPEN
#undef DL_CLOSE
#undef DL_SYM

#endif // ENGINE_DLL_LOADER_H_
