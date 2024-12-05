#ifndef OBJ_TYPES_H_
#define OBJ_TYPES_H_

#include <string>
#include <vector>
#include <functional>

namespace obj {

struct Indices {
  unsigned int fv;
  unsigned int fn;
  unsigned int ft;

  bool operator==(const Indices& other) const noexcept {
    return other.fv == fv && other.fn == fn && other.ft == ft;
  }

  struct Hash {
    size_t operator()(const Indices& idx) const {
      return std::hash<unsigned int>()(idx.fv) ^
             std::hash<unsigned int>()(idx.fn) ^
             std::hash<unsigned int>()(idx.ft);
    }
  };
};

struct NewMtl {
  std::string name;
  std::string map_ka;
  std::string map_kd;
  std::string map_ks;

  float Ns = 32.0f;
  float d = 1.0f;
  float Ka[3] = {};
  float Kd[3] = {0.7f, 0.7f, 0.7f};
  float Ks[3] = {};
  float Ke[3] = {};
};

struct UseMtl {
  unsigned int index;
  unsigned int offset;
};

struct Data {
  std::string dir_path;

  std::vector<float> vn;
  std::vector<float> vt;
  std::vector<float> v;

  std::vector<Indices> indices;
  std::vector<UseMtl> usemtl;
  std::vector<NewMtl> mtl;
};

} // namespace obj

#endif // OBJ_TYPES_H_
