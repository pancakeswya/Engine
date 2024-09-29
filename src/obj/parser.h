#ifndef OBJ_PARSER_H_
#define OBJ_PARSER_H_

#include <string>
#include <vector>
#include <stdexcept>

namespace obj {

struct Error final : std::runtime_error {
  using runtime_error::runtime_error;
};

struct Index {
  unsigned int fv;
  unsigned int fn;
  unsigned int ft;
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

  std::vector<Index> indices;
  std::vector<UseMtl> usemtl;
  std::vector<NewMtl> mtl;
};

Data ParseFromFile(const std::string& path);

} // namespace obj

#endif // OBJ_PARSER_H_