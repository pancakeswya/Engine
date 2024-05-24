#include "base/io.h"

#include <fstream>

namespace io {

namespace {

long int FileSize(std::ifstream& file) {
  const long int p = file.tellg();
  file.seekg(0, std::ifstream::end);
  const long int n = file.tellg();
  file.seekg(p, std::ifstream::beg);
  if (n > 0) {
    return n;
  }
  return 0;
}

} // namespace

std::vector<char> ReadFileBytes(const std::string& path) {
  std::ifstream file(path, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    THROW_UNEXPECTED("failed to open file");
  }
  size_t fileSize = (size_t) file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}


} // namespace io