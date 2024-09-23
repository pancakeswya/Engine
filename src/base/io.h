#ifndef BASE_IO_H_
#define BASE_IO_H_

#include <vector>
#include <string>
#include <stdexcept>

namespace io {

struct Error final : std::runtime_error {
  using std::runtime_error::runtime_error;
};

extern std::vector<char> ReadFile(const std::string& path);

} // namespace io

#endif // BASE_IO_H_