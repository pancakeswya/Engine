#include "base/io.h"

#include <fstream>

namespace io {

std::vector<char> ReadFile(const std::string& path) {
  std::ifstream file(path, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    throw Error("failed to open file");
  }
  const auto file_sz = static_cast<std::streamsize>(file.tellg());
  std::vector<char> buffer(file_sz);

  file.seekg(0);
  file.read(buffer.data(), file_sz);

  return buffer;
}


} // namespace io