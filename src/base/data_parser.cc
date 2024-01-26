#include "base/data_parser.h"

#include <cstring>
#include <filesystem>
#include <fstream>

#include "third_party/earcut.h"

namespace engine::DataParser {

namespace {

constexpr size_t kBufferSize = 65536;

inline bool IsSpace(char c) noexcept {
  return (c == ' ') || (c == '\t') || (c == '\r');
}

inline bool IsDigit(char c) noexcept { return (c >= '0') && (c <= '9'); }

inline bool IsEndOfName(char c) noexcept {
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

long int FileSize(std::ifstream& file) {
  long int p, n;
  p = file.tellg();
  file.seekg(0, std::ifstream::end);
  n = file.tellg();
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

inline const char* ReadMtlSingle(const char* ptr, float& mtl) noexcept {
  char* end = nullptr;
  mtl = std::strtof(ptr, &end);
  return end;
}

inline const char* ReadMtlTriple(const char* ptr, float triple[3]) noexcept {
  ptr = ReadMtlSingle(ptr, triple[0]);
  ptr = ReadMtlSingle(ptr, triple[1]);
  ptr = ReadMtlSingle(ptr, triple[2]);

  return ptr;
}

struct Point3D {
  float x, y, z;
};

inline float GetLength(const Point3D& e) noexcept {
  return std::sqrt(e.x * e.x + e.y * e.y + e.z * e.z);
}

inline Point3D Cross(const Point3D& v1, const Point3D& v2) noexcept {
  return {v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z,
          v1.x * v2.y - v1.y * v2.x};
}

inline Point3D Normalize(const Point3D& e) noexcept {
  float inv_length = 1.0f / GetLength(e);
  return {e.x * inv_length, e.y * inv_length, e.z * inv_length};
}

inline float Dot(const Point3D& v1, const Point3D& v2) noexcept {
  return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

inline Point3D WorldToLocal(const Point3D& a, const Point3D& u,
                            const Point3D& v, const Point3D& w) noexcept {
  return {Dot(a, u), Dot(a, v), Dot(a, w)};
}

Status ProcessPolygon(Data& data, const std::vector<Index>& raw_ind,
                    unsigned int npolys) {
  // quad to 2 triangles
  if (npolys == 4) {
    auto vi0 = size_t(raw_ind[0].fv);
    auto vi1 = size_t(raw_ind[1].fv);
    auto vi2 = size_t(raw_ind[2].fv);
    auto vi3 = size_t(raw_ind[3].fv);

    if (((3 * vi0 + 2) >= data.v.size()) || ((3 * vi1 + 2) >= data.v.size()) ||
        ((3 * vi2 + 2) >= data.v.size()) || ((3 * vi3 + 2) >= data.v.size())) {
      return Status::kInvalidFile;
    }
    float v0x = data.v[vi0 * 3 + 0];
    float v0y = data.v[vi0 * 3 + 1];
    float v0z = data.v[vi0 * 3 + 2];
    float v1x = data.v[vi1 * 3 + 0];
    float v1y = data.v[vi1 * 3 + 1];
    float v1z = data.v[vi1 * 3 + 2];
    float v2x = data.v[vi2 * 3 + 0];
    float v2y = data.v[vi2 * 3 + 1];
    float v2z = data.v[vi2 * 3 + 2];
    float v3x = data.v[vi3 * 3 + 0];
    float v3y = data.v[vi3 * 3 + 1];
    float v3z = data.v[vi3 * 3 + 2];

    float e02x = v2x - v0x;
    float e02y = v2y - v0y;
    float e02z = v2z - v0z;
    float e13x = v3x - v1x;
    float e13y = v3y - v1y;
    float e13z = v3z - v1z;

    float sqr02 = e02x * e02x + e02y * e02y + e02z * e02z;
    float sqr13 = e13x * e13x + e13y * e13y + e13z * e13z;
    // find nearest edge
    if (sqr02 < sqr13) {
      data.indices.push_back(raw_ind[0]);
      data.indices.push_back(raw_ind[1]);
      data.indices.push_back(raw_ind[2]);
      data.indices.push_back(raw_ind[0]);
      data.indices.push_back(raw_ind[2]);
      data.indices.push_back(raw_ind[3]);
    } else {
      data.indices.push_back(raw_ind[0]);
      data.indices.push_back(raw_ind[1]);
      data.indices.push_back(raw_ind[3]);
      data.indices.push_back(raw_ind[1]);
      data.indices.push_back(raw_ind[2]);
      data.indices.push_back(raw_ind[3]);
    }
    // triangulate polygon with earcut method
  } else if (npolys > 4) {
    Index i0{}, i0_2{};
    // Find the normal axis of the polygon using Newell's method
    Point3D n1{};
    for (size_t k = 0; k < npolys; ++k) {
      i0 = raw_ind[k % npolys];
      auto vi0 = size_t(i0.fv);

      size_t j = (k + 1) % npolys;
      i0_2 = raw_ind[j];
      auto vi0_2 = size_t(i0_2.fv);

      float v0x = data.v[vi0 * 3 + 0];
      float v0y = data.v[vi0 * 3 + 1];
      float v0z = data.v[vi0 * 3 + 2];

      float v0x_2 = data.v[vi0_2 * 3 + 0];
      float v0y_2 = data.v[vi0_2 * 3 + 1];
      float v0z_2 = data.v[vi0_2 * 3 + 2];

      const Point3D point1 = {v0x, v0y, v0z};
      const Point3D point2 = {v0x_2, v0y_2, v0z_2};

      Point3D a = {point1.x - point2.x, point1.y - point2.y,
                   point1.z - point2.z};
      Point3D b = {point1.x + point2.x, point1.y + point2.y,
                   point1.z + point2.z};

      n1.x += (a.y * b.z);
      n1.y += (a.z * b.x);
      n1.z += (a.x * b.y);
    }
    float length_n = GetLength(n1);
    // Check if zero length normal
    if (length_n <= 0) {
      return Status::kInvalidFile;
    }
    // Negative is to flip the normal to the correct direction
    float inv_length = -1.0f / length_n;
    n1.x *= inv_length;
    n1.y *= inv_length;
    n1.z *= inv_length;

    Point3D axis_w, axis_v, axis_u;
    axis_w = n1;
    Point3D a;
    if (std::abs(axis_w.x) > 0.9999999f) {
      a = {0.0f, 1.0f, 0.0f};
    } else {
      a = {1.0f, 0.0f, 0.0f};
    }
    axis_v = Normalize(Cross(axis_w, a));
    axis_u = Cross(axis_w, axis_v);
    using Point2D = std::pair<float, float>;

    // first polyline define the main polygon
    std::vector<std::vector<Point2D>> polygon;

    std::vector<Point2D> polyline;

    // Find best normal and project v0x and v0y to those coordinates, instead of
    // picking a plane aligned with an axis (which can flip polygons).

    // Fill polygon data(facevarying vertices).
    for (size_t k = 0; k < npolys; ++k) {
      i0 = raw_ind[k];
      auto vi0 = size_t(i0.fv);
      if (3 * vi0 + 2 >= data.v.size()) {
        return Status::kInvalidFile;
      }

      float v0x = data.v[vi0 * 3 + 0];
      float v0y = data.v[vi0 * 3 + 1];
      float v0z = data.v[vi0 * 3 + 2];

      Point3D polypoint = {v0x, v0y, v0z};
      Point3D loc = WorldToLocal(polypoint, axis_u, axis_v, axis_w);

      polyline.emplace_back(loc.x, loc.y);
    }
    polygon.push_back(polyline);
    std::vector<unsigned int> order = mapbox::earcut(polygon);
    if (order.size() % 3 != 0) {
      return Status::kInvalidFile;
    }
    for (unsigned int idx : order) {
      data.indices.push_back(raw_ind[idx]);
    }
  } else {
    // polygon is a triangle just move it in
    std::move(raw_ind.begin(), raw_ind.end(), std::back_inserter(data.indices));
  }
  return Status::kNoExc;
}

std::pair<Status, const char*> ParseVertex(const char* ptr, std::vector<float>& verts) {
  float vert;
  char* end = nullptr;
  char type = *ptr++;
  // texture has 2d coords everything else 3
  int vert_max = (type == 't') ? 2 : 3;
  for (int i = 0; i < vert_max; ++i) {
    vert = std::strtof(ptr, &end);
    if (end == ptr) {
      return {Status::kInvalidFile, nullptr};
    }
    verts.push_back(vert);
    ptr = SkipSpace(end);
  }
  return {Status::kNoExc, ptr};
}

std::pair<Status, const char*> ParseFacet(const char* ptr, Data& data) {
  char* end = nullptr;
  long int tmp_i;
  std::vector<Index> raw_ind;
  size_t npolys = 0;
  while (*ptr != '\n') {
    Index idx = {};
    tmp_i = std::strtol(ptr, &end, 10);
    if (end == ptr || tmp_i == 0) {
      return {Status::kInvalidFile, nullptr};
    } else if (tmp_i < 0) {
      idx.fv = data.v.size() / 3 - static_cast<unsigned int>(-tmp_i);
    } else if (tmp_i > 0) {
      idx.fv = static_cast<unsigned int>(tmp_i) - 1;
    }
    ptr = end;
    if (*ptr == '/') {
      ++ptr;
      if (IsDigit(*ptr)) {
        tmp_i = std::strtol(ptr, &end, 10);
        if (end == ptr || tmp_i == 0) {
          return {Status::kInvalidFile, nullptr};
        } else if (tmp_i < 0) {
          idx.ft = data.vt.size() / 2 - static_cast<unsigned int>(-tmp_i);
        } else if (tmp_i > 0) {
          idx.ft = static_cast<unsigned int>(tmp_i) - 1;
        }
        ptr = end;
      }
    }
    if (*ptr == '/') {
      tmp_i = std::strtol(++ptr, &end, 10);
      if (end == ptr || tmp_i == 0) {
        return {Status::kInvalidFile, nullptr};
      } else if (tmp_i < 0) {
        idx.fn = data.vn.size() / 3 - static_cast<unsigned int>(-tmp_i);
      } else if (tmp_i > 0) {
        idx.fn = static_cast<unsigned int>(tmp_i) - 1;
      }
      ptr = end;
    }
    // push parsed indices
    raw_ind.push_back(idx);
    ptr = SkipSpace(ptr);
    ++npolys;
  }
  Status stat = ProcessPolygon(data, raw_ind, npolys);
  return {stat, ptr};
}

NewMtl DefaultMtl() noexcept {
  return {.name = "",
          .map_ka = "",
          .map_kd = "",
          .map_ks = "",
          .Ns = 32.0f,
          .d = 1.0f,
          .Ka = {},
          .Kd = {0.7f, 0.7f, 0.7f},
          .Ks = {},
          .Ke = {}};
}

const char* ParseMtl(const char* p, Data& data) {
  std::string path_mtl = GetName(&p);
  std::ifstream mtl_file(data.dir_path + path_mtl, std::ifstream::binary);
  if (mtl_file.is_open()) {
    auto new_mtl = DefaultMtl();
    bool found_d = false;

    long int bytes = FileSize(mtl_file);
    auto buf = new char[bytes + 1]{};
    mtl_file.read(buf, bytes);
    unsigned int read = mtl_file.gcount();
    buf[read] = '\0';

    const char* ptr = buf;
    const char* eof = buf + read;

    while (ptr < eof) {
      ptr = SkipSpace(ptr);
      switch (*ptr) {
        case 'n':
          ptr++;
          if (ptr[0] == 'e' && ptr[1] == 'w' && ptr[2] == 'm' &&
              ptr[3] == 't' && ptr[4] == 'l' && IsSpace(ptr[5])) {
            if (!new_mtl.name.empty()) {
              data.mtl.push_back(std::move(new_mtl));
              new_mtl = DefaultMtl();
            }
            ptr += 5;
            new_mtl.name = GetName(&ptr);
          }
          break;
        case 'K':
          if (ptr[1] == 'a') {
            ptr = ReadMtlTriple(ptr + 2, new_mtl.Ka);
          } else if (ptr[1] == 'd') {
            ptr = ReadMtlTriple(ptr + 2, new_mtl.Kd);
          } else if (ptr[1] == 's') {
            ptr = ReadMtlTriple(ptr + 2, new_mtl.Ks);
          } else if (ptr[1] == 'e') {
            ptr = ReadMtlTriple(ptr + 2, new_mtl.Ke);
          }
          break;
        case 'N':
          if (ptr[1] == 's') {
            ptr = ReadMtlSingle(ptr + 2, new_mtl.Ns);
          }
          break;
        case 'T':
          if (ptr[1] == 'r') {
            float Tr;
            ptr = ReadMtlSingle(ptr + 2, Tr);
            if (!found_d) {
              new_mtl.d = 1.0f - Tr;
            }
          }
          break;
        case 'd':
          if (IsSpace(ptr[1])) {
            ptr = ReadMtlSingle(ptr + 1, new_mtl.d);
            found_d = true;
          }
          break;
        case 'm':
          ptr++;
          if (ptr[0] == 'a' && ptr[1] == 'p' && ptr[2] == '_') {
            ptr += 3;
            std::string* map_ptr = nullptr;
            if (*ptr == 'K') {
              ptr++;
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
          break;
      }
      ptr = SkipLine(ptr);
    }
    if (!new_mtl.name.empty()) {
      data.mtl.push_back(new_mtl);
    }
    delete[] buf;
    mtl_file.close();
  }
  return p;
}

const char* ParseUsemtl(const char* ptr, Data& data) {
  std::string use_mtl_name = GetName(&ptr);
  for (unsigned int i = 0; i < data.mtl.size(); ++i) {
    if (data.mtl[i].name == use_mtl_name) {
      data.usemtl.push_back({i, 0, 0});
      if (!data.indices.empty()) {
        data.usemtl[data.usemtl.size() - 2].offset_fv = data.indices.size();
      }
      break;
    }
  }
  return ptr;
}

Status ParseBuffer(const char* ptr, const char* end, Data& data) {
  while (ptr != end) {
    std::pair<Status, const char*> parse_res;
    ptr = SkipSpace(ptr);
    if (*ptr == 'v') {
      ++ptr;
      if (*ptr == ' ' || *ptr == '\t') {
        parse_res = ParseVertex(ptr, data.v);
      } else if (*ptr == 'n') {
        parse_res = ParseVertex(ptr, data.vn);
      } else if (*ptr == 't') {
        parse_res = ParseVertex(ptr, data.vt);
      }
    } else if (*ptr == 'f') {
      ++ptr;
      if (*ptr == ' ' || *ptr == '\t') {
        parse_res = ParseFacet(ptr, data);
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
    if (auto[stat, next_ptr] = parse_res; stat != Status::kNoExc) {
      return stat;
    } else {
      ptr = next_ptr;
    }
    ptr = SkipLine(ptr);
  }
  return Status::kNoExc;
}

Status ReadFile(std::string_view path, Data& data) {
  std::ifstream file(path.data(), std::ifstream::binary);
  if (!file.is_open()) {
    return Status::kNoFile;
  }
  unsigned int read, bytes;
  char *start, *end, *last;
  {
    std::filesystem::path p(path);
    p.remove_filename();
    data.dir_path = p.generic_string();
  }
  Status stat = Status::kNoExc;
  auto buffer = new char[2 * kBufferSize]{};
  start = buffer;
  for (;;) {
    file.read(start, kBufferSize);
    read = file.gcount();
    if (!read && start == buffer) {
      break;
    }
    if (!read || (read < kBufferSize && start[read - 1] != '\n')) {
      start[read++] = '\n';
    }
    end = start + read;
    if (end == buffer) {
      break;
    }
    last = end;
    while (last > buffer) {
      --last;
      if (*last == '\n') {
        break;
      }
    }
    if (*last != '\n') {
      break;
    }
    ++last;
    if (stat = ParseBuffer(buffer, last, data); stat != Status::kNoExc) {
      break;
    }
    bytes = static_cast<unsigned int>(end - last);
    std::memmove(buffer, last, bytes);
    start = buffer + bytes;
  }
  if (data.mtl.empty()) {
    data.mtl.emplace_back(DefaultMtl());
  }
  if (data.usemtl.empty()) {
    data.usemtl.emplace_back();
  }
  data.usemtl.back().offset_fv = data.indices.size();
  delete[] buffer;
  file.close();

  return Status::kNoExc;
}

inline void Flush(Data* data) noexcept {
  std::vector<NewMtl>().swap(data->mtl);
  std::vector<UseMtl>().swap(data->usemtl);
  std::vector<Index>().swap(data->indices);

  std::vector<float>().swap(data->vn);
  std::vector<float>().swap(data->vt);
  std::vector<float>().swap(data->v);

  std::string().swap(data->dir_path);
}

}  // namespace

std::pair<Data*, Status> FromFile(std::string_view path) {
  auto data = new Data;

  Status stat = ReadFile(path, *data);
  if (stat != Status::kNoExc) {
    Flush(data);
  }

  return {data, stat};
}

}  // namespace objv::DataParser
