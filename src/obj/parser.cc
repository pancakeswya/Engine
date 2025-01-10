#include "obj/parser.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <cmath>

#include <glm/glm.hpp>

#include "obj/error.h"
#include "mapbox/earcut.hpp"

namespace obj {

namespace {

constexpr size_t kBufferSize = 65536;

inline std::string GetDirPath(const std::string& path) {
  std::filesystem::path p(path);
  p.remove_filename();
  return p.generic_string();
}

inline bool IsSpace(const char c) noexcept {
  return (c == ' ') || (c == '\t') || (c == '\r');
}

inline bool IsDigit(const char c) noexcept { return (c >= '0') && (c <= '9'); }

inline bool IsEndOfName(const char c) noexcept {
  return (c == '\t') || (c == '\r') || (c == '\n');
}

const char* SkipSpace(const char* ptr) noexcept {
  for (; IsSpace(*ptr); ++ptr)
    ;
  return ptr;
}

const char* SkipLine(const char* ptr) noexcept {
  for (; *ptr != '\n'; ++ptr)
    ;
  return ++ptr;
}

std::streamsize FileSize(std::ifstream& file) {
  const long int p = file.tellg();
  file.seekg(0, std::ifstream::end);
  const long int n = file.tellg();
  file.seekg(p, std::ifstream::beg);
  if (n > 0) {
    return n;
  }
  return 0;
}

std::string GetName(const char** ptr) {
  const char* p = *ptr;
  p = SkipSpace(p);
  std::string name;
  for (; !IsEndOfName(*p); ++p) {
    name += *p;
  }
  *ptr = p;
  return name;
}

template<int count>
inline const char* ReadMtl(const char* ptr, float* mtl) noexcept {
  char* end = nullptr;
  *mtl = std::strtof(ptr, &end);
  return ReadMtl<count - 1>(end, mtl + 1);
}

template<>
inline const char* ReadMtl<0>(const char* ptr, [[maybe_unused]]float* mtl) noexcept {
  return ptr;
}

void ProcessPolygon(Data& data, const std::vector<Indices>& raw_indices) {
  // quad to 2 triangles
  if (const size_t indices_len = raw_indices.size(); indices_len == 4) {
    const unsigned int vi0 = raw_indices[0].fv;
    const unsigned int vi1 = raw_indices[1].fv;
    const unsigned int vi2 = raw_indices[2].fv;
    const unsigned int vi3 = raw_indices[3].fv;

    if (((3 * vi0 + 2) >= data.v.size()) || ((3 * vi1 + 2) >= data.v.size()) ||
        ((3 * vi2 + 2) >= data.v.size()) || ((3 * vi3 + 2) >= data.v.size())) {
      throw Error("invalid obj model");
    }
    const glm::vec3 v0 = {data.v[vi0 * 3 + 0], data.v[vi0 * 3 + 1], data.v[vi0 * 3 + 2] };
    const glm::vec3 v1 = { data.v[vi1 * 3 + 0], data.v[vi1 * 3 + 1], data.v[vi1 * 3 + 2] };
    const glm::vec3 v2 = { data.v[vi2 * 3 + 0], data.v[vi2 * 3 + 1], data.v[vi2 * 3 + 2] };
    const glm::vec3 v3 = { data.v[vi3 * 3 + 0], data.v[vi3 * 3 + 1], data.v[vi3 * 3 + 2] };

    const glm::vec3 e02 = v2 - v0;
    const glm::vec3 e13 = v3 - v1;
    // find nearest edge
    data.indices.push_back(raw_indices[0]);
    data.indices.push_back(raw_indices[1]);
    if (glm::dot(e02, e02) < glm::dot(e13, e13)) {
      data.indices.push_back(raw_indices[2]);
      data.indices.push_back(raw_indices[0]);
    } else {
      data.indices.push_back(raw_indices[3]);
      data.indices.push_back(raw_indices[1]);
    }
    data.indices.push_back(raw_indices[2]);
    data.indices.push_back(raw_indices[3]);
  } else if (indices_len > 4) {
    glm::vec3 n1 = {};
    for (size_t k = 0; k < indices_len; ++k) {
      const unsigned int vi1 = raw_indices[k].fv;
      const unsigned int vi2 = raw_indices[(k + 1) % indices_len].fv;

      const glm::vec3 point1 = { data.v[vi1 * 3 + 0], data.v[vi1 * 3 + 1], data.v[vi1 * 3 + 2] };
      const glm::vec3 point2 = { data.v[vi2 * 3 + 0], data.v[vi2 * 3 + 1], data.v[vi2 * 3 + 2] };

      const glm::vec3 a = point1 - point2;
      const glm::vec3 b = point1 + point2;

      n1.x += a.y * b.z;
      n1.y += a.z * b.x;
      n1.z += a.x * b.y;
    }
    const float length_n = glm::length(n1);
    if (length_n <= 0) {
      throw Error("Invalid obj model");
    }
    const glm::vec3 axis_w = n1 * (-1.0f / length_n);

    glm::vec3 a = {};
    if (std::abs(axis_w.x) > 0.9999999f) a.y = 1.0f; else a.x = 1.0f;

    const glm::vec3 axis_v = glm::normalize(glm::cross(axis_w, a));
    const glm::vec3 axis_u = glm::cross(axis_w, axis_v);

    using Point2D = std::pair<float, float>;

    std::vector<std::vector<Point2D>> polygon;
    std::vector<Point2D> polyline;

    for (const Indices& indices : raw_indices) {
      const unsigned int vi0 = indices.fv;
      if (3 * vi0 + 2 >= data.v.size()) {
        throw Error("invalid model file");
      }
      glm::vec3 polypoint = {data.v[vi0 * 3 + 0], data.v[vi0 * 3 + 1], data.v[vi0 * 3 + 2]};

      polyline.emplace_back(glm::dot(polypoint, axis_u), glm::dot(polypoint, axis_v));
    }
    polygon.push_back(std::move(polyline));
    std::vector order = mapbox::earcut(polygon);
    if (order.size() % 3 != 0) {
      throw Error("invalid obj model");
    }
    for (const auto idx : order) {
      data.indices.push_back(raw_indices[idx]);
    }
  } else {
    std::move(raw_indices.begin(), raw_indices.end(), std::back_inserter(data.indices));
  }
}

template<int count>
const char* ParseVertex(const char* ptr, std::vector<float>& verts) {
  char* end = nullptr;

  for (int i = 0; i < count; ++i) {
    const float vert = std::strtof(ptr, &end);
    if (end == ptr) {
     throw Error("invalid file verices");
    }
    verts.push_back(vert);
    ptr = SkipSpace(end);
  }
  return ptr;
}

const char* ParseFacet(const char* ptr, Data& data) {
  char* end = nullptr;

  std::vector<Indices> raw_indices;
  while (*ptr != '\n') {
    Indices indices = {};
    long int index = std::strtol(ptr, &end, 10);
    if (end == ptr || index == 0) {
      throw Error("failed to parse facet");
    } else if (index < 0) {
      indices.fv = data.v.size() / 3 - static_cast<unsigned int>(-index);
    } else if (index > 0) {
      indices.fv = static_cast<unsigned int>(index) - 1;
    }
    ptr = end;
    if (*ptr == '/') {
      ++ptr;
      if (IsDigit(*ptr)) {
        index = std::strtol(ptr, &end, 10);
        if (end == ptr || index == 0) {
          throw Error("invalid separator in facet");
        }
        if (index < 0) {
          indices.ft = data.vt.size() / 2 - static_cast<unsigned int>(-index);
        } else if (index > 0) {
          indices.ft = static_cast<unsigned int>(index) - 1;
        }
        ptr = end;
      }
    }
    if (*ptr == '/') {
      index = std::strtol(++ptr, &end, 10);
      if (end == ptr || index == 0) {
        throw Error("invalid seporator in facet");
      }
      if (index < 0) {
        indices.fn = data.vn.size() / 3 - static_cast<unsigned int>(-index);
      } else if (index > 0) {
        indices.fn = static_cast<unsigned int>(index) - 1;
      }
      ptr = end;
    }
    raw_indices.push_back(indices);
    ptr = SkipSpace(ptr);
  }
  ProcessPolygon(data, raw_indices);
  return ptr;
}

void ParseMtlFile(std::ifstream& mtl_file, Data& data) {
  NewMtl new_mtl;
  bool found_d = false;

  const std::streamsize bytes = FileSize(mtl_file);
  std::vector<char> buffer(static_cast<size_t>(bytes + 1));

  mtl_file.read(buffer.data(), bytes);
  const unsigned int read = mtl_file.gcount();

  const char* buffer_ptr = buffer.data();

  const char* ptr = buffer_ptr;
  const char* eof = buffer_ptr + read;

  while (ptr < eof) {
    ptr = SkipSpace(ptr);
    switch (*ptr) {
      case 'n':
        ++ptr;
        if (ptr[0] == 'e' && ptr[1] == 'w' && ptr[2] == 'm' &&
            ptr[3] == 't' && ptr[4] == 'l' && IsSpace(ptr[5])) {
          if (!new_mtl.name.empty()) {
            data.mtl.push_back(std::move(new_mtl));
            new_mtl = NewMtl();
          }
          ptr += 5;
          new_mtl.name = GetName(&ptr);
        }
        break;
      case 'K':
        if (ptr[1] == 'a') {
          ptr = ReadMtl<3>(ptr + 2, new_mtl.Ka);
        } else if (ptr[1] == 'd') {
          ptr = ReadMtl<3>(ptr + 2, new_mtl.Kd);
        } else if (ptr[1] == 's') {
          ptr = ReadMtl<3>(ptr + 2, new_mtl.Ks);
        } else if (ptr[1] == 'e') {
          ptr = ReadMtl<3>(ptr + 2, new_mtl.Ke);
        }
        break;
      case 'N':
        if (ptr[1] == 's') {
          ptr = ReadMtl<1>(ptr + 2, &new_mtl.Ns);
        }
        break;
      case 'T':
        if (ptr[1] == 'r') {
          float Tr;
          ptr = ReadMtl<1>(ptr + 2, &Tr);
          if (!found_d) {
            new_mtl.d = 1.0f - Tr;
          }
        }
        break;
      case 'd':
        if (IsSpace(ptr[1])) {
          ptr = ReadMtl<1>(ptr + 1, &new_mtl.d);
          found_d = true;
        }
        break;
      case 'm':
        ++ptr;
        if (ptr[0] == 'a' && ptr[1] == 'p' && ptr[2] == '_') {
          ptr += 3;
          std::string* map_ptr = nullptr;
          if (*ptr == 'K') {
            ++ptr;
            if (ptr[0] == 'a' && IsSpace(ptr[1])) {
              ++ptr;
              new_mtl.map_ka = GetName(&ptr);
              map_ptr = &new_mtl.map_ka;
            } else if (ptr[0] == 'd' && IsSpace(ptr[1])) {
              ++ptr;
              new_mtl.map_kd = GetName(&ptr);
              map_ptr = &new_mtl.map_kd;
            } else if (ptr[0] == 's' && IsSpace(ptr[1])) {
              ++ptr;
              new_mtl.map_ks = GetName(&ptr);
              map_ptr = &new_mtl.map_ks;
            }
          }
          if (map_ptr && std::filesystem::path(*map_ptr).is_relative()) {
            *map_ptr = data.dir_path + *map_ptr;
          }
        }
        break;
      case '#':
      default:
        break;
    }
    ptr = SkipLine(ptr);
  }
  if (!new_mtl.name.empty()) {
    data.mtl.push_back(new_mtl);
  }
}

inline const char* ParseMtl(const char* ptr, Data& data) {
  const std::string path_mtl = GetName(&ptr);
  std::ifstream mtl_file(data.dir_path + path_mtl, std::ifstream::binary);
  if (mtl_file.is_open()) {
    ParseMtlFile(mtl_file, data);
  }
  return ptr;
}

const char* ParseUsemtl(const char* ptr, Data& data) {
  std::string use_mtl_name = GetName(&ptr);
  for (unsigned int i = 0; i < data.mtl.size(); ++i) {
    if (data.mtl[i].name == use_mtl_name) {
      data.usemtl.push_back({i, 0});
      if (!data.indices.empty()) {
        data.usemtl[data.usemtl.size() - 2].offset = data.indices.size();
      }
      break;
    }
  }
  return ptr;
}

void ParseBuffer(const char* ptr, const char* end, Data& data) {
  while (ptr != end) {
    ptr = SkipSpace(ptr);
    if (*ptr == 'v') {
      ++ptr;
      if (*ptr == ' ' || *ptr == '\t') {
        ptr = ParseVertex<3>(++ptr, data.v);
      } else if (*ptr == 'n') {
        ptr = ParseVertex<3>(++ptr, data.vn);
      } else if (*ptr == 't') {
        ptr = ParseVertex<2>(++ptr, data.vt);
      }
    } else if (*ptr == 'f') {
      ++ptr;
      if (*ptr == ' ' || *ptr == '\t') {
        ptr = ParseFacet(ptr, data);
      }
    } else if (*ptr == 'm') {
      ++ptr;
      if (ptr[0] == 't' && ptr[1] == 'l' && ptr[2] == 'l' && ptr[3] == 'i' &&
          ptr[4] == 'b' && IsSpace(ptr[5])) {
        ptr = ParseMtl(ptr + 6, data);
      }
    } else if (*ptr == 'u') {
      ++ptr;
      if (ptr[0] == 's' && ptr[1] == 'e' && ptr[2] == 'm' && ptr[3] == 't' &&
          ptr[4] == 'l' && IsSpace(ptr[5])) {
        ptr = ParseUsemtl(ptr + 6, data);
      }
    }
    ptr = SkipLine(ptr);
  }
}

}  // namespace

Data ParseFromFile(const std::string& path) {
  Data data = {};
  std::ifstream file(path.data(), std::ifstream::binary);
  if (!file.is_open()) {
    throw Error("model file is not found");
  }
  data.dir_path = GetDirPath(path);

  std::vector<char> buffer(2 * kBufferSize);
  char* buffer_ptr = buffer.data();
  char* start = buffer_ptr;
  for (;;) {
    file.read(start, kBufferSize);
    unsigned int read = file.gcount();
    if (!read && start == buffer_ptr) {
      break;
    }
    if (!read || (read < kBufferSize && start[read - 1] != '\n')) {
      start[read++] = '\n';
    }
    char *end = start + read;
    if (end == buffer_ptr) {
      break;
    }
    char *last = end;
    while (last > buffer_ptr) {
      --last;
      if (*last == '\n') {
        break;
      }
    }
    if (*last != '\n') {
      break;
    }
    ++last;
    ParseBuffer(buffer_ptr, last, data);
    const auto bytes = static_cast<unsigned int>(end - last);
    std::memmove(buffer_ptr, last, bytes);
    start = buffer_ptr + bytes;
  }
  if (data.mtl.empty()) {
    data.mtl.emplace_back();
  }
  if (data.usemtl.empty()) {
    data.usemtl.emplace_back();
  }
  data.usemtl.back().offset = data.indices.size();

  return data;
}

} // namespace obj