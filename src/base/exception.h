#ifndef BASE_EXCEPTION_H_
#define BASE_EXCEPTION_H_

#include <exception>
#include <string>

namespace engine {

class Exception : public std::exception {
 public:
  explicit
  Exception(std::string message) noexcept : message_ (std::move(message)) {}
  ~Exception() noexcept override = default;
  [[nodiscard]]
  const char *what() const noexcept override {
    return message_.c_str();
  }
 private:
  std::string message_;
};

} // namespace engine

#define THROW_UNEXPECTED(message) throw Exception(message + std::string(" [FILE: ") + __FILE__ + " LINE: " + std::to_string(__LINE__) + ']')

#endif // BASE_EXCEPTION_H_