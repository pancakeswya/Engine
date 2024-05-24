#ifndef io_h_
#define io_h_

#include "base/exception.h"

#include <vector>
#include <string>

namespace io {

class Exception : public engine::Exception {
public:
  explicit Exception(const std::string& message)
    : engine::Exception("io error: " + message) {}
  ~Exception() override = default;
};

std::vector<char> ReadFileBytes(const std::string& path);

} // namespace io

#endif // io_h_