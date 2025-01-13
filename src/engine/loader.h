#ifndef ENGINE_LOADER_H_
#define ENGINE_LOADER_H_

#include <dlfcn.h>

#include <string>

class Loader {
public:
  explicit Loader(const std::string& path);
  virtual ~Loader();

  template <typename T>
  T GetProc(const std::string& path) const;
protected:
  void* handle_;
};

inline Loader::Loader(const std::string& path) {
  handle_ = dlopen(path.c_str(), RTLD_NOW);
  if (handle_ == nullptr) {
    throw std::runtime_error("Failed to load library");
  }
}

inline Loader::~Loader() {
  dlclose(handle_);
}

template <typename T>
inline T Loader::GetProc(const std::string& path) const {
  void* proc = dlsym(handle_, path.c_str());
  if (proc == nullptr) {
    throw std::runtime_error("Failed to load symbol '" + path + "'");
  }
  return reinterpret_cast<T>(proc);
}


#endif // ENGINE_LOADER_H_
