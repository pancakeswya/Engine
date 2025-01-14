#ifndef ENGINE_DLL_LOADER_H_
#define ENGINE_DLL_LOADER_H_

#include <dlfcn.h>

#include <string>

namespace engine {

class DllLoader {
public:
  explicit DllLoader(const std::string& path);
  virtual ~DllLoader();

  template <typename T>
  T Load(const std::string& path) const;
protected:
  void* handle_;
};

inline DllLoader::DllLoader(const std::string& path) {
  if (handle_ = dlopen(path.c_str(), RTLD_NOW); handle_ == nullptr) {
    throw std::runtime_error("Failed to load library: " + path);
  }
}

inline DllLoader::~DllLoader() {
  dlclose(handle_);
}

template <typename T>
inline T DllLoader::Load(const std::string& path) const {
  void* proc = dlsym(handle_, path.c_str());
  if (proc == nullptr) {
    throw std::runtime_error("Failed to load symbol '" + path + "'");
  }
  return reinterpret_cast<T>(proc);
}

} // namespace engine

#endif // ENGINE_DLL_LOADER_H_
