#ifndef ENGINE_DLL_ERROR_H_
#define ENGINE_DLL_ERROR_H_

#include <stdexcept>

class Error final : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

#endif //  ENGINE_DLL_ERROR_H_
