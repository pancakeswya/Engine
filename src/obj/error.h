#ifndef OBJ_ERROR_H_
#define OBJ_ERROR_H_

#include <stdexcept>

namespace obj {

struct Error final : std::runtime_error {
  using runtime_error::runtime_error;
};

} // namespace obj

#endif // OBJ_ERROR_H_