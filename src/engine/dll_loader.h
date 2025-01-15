#ifndef ENGINE_DLL_LOADER_H_
#define ENGINE_DLL_LOADER_H_

#include "engine/error.h"

#include <dlfcn.h>

#include <string>

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
  T Load(const std::string& path) const;
protected:
  void* handle_;
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
  if (handle_ = dlopen(path.c_str(), RTLD_NOW); handle_ == nullptr) {
    throw Error("Failed to load library: " + path);
  }
}

inline DllLoader::~DllLoader() {
  if (handle_ != nullptr) {
    dlclose(handle_);
    handle_ = nullptr;
  }
}

template <typename T>
inline T DllLoader::Load(const std::string& path) const {
  void* proc = dlsym(handle_, path.c_str());
  if (proc == nullptr) {
    throw Error("Failed to load symbol '" + path + "'");
  }
  return reinterpret_cast<T>(proc);
}

} // namespace engine

#endif // ENGINE_DLL_LOADER_H_
