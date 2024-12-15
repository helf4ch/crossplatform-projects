#pragma once 

#include <exception>
#include <string>

namespace my::common {

#ifdef _WIN32
typedef unsigned long pid_t;
typedef unsigned long return_code_t;
#else
typedef int pid_t;
typedef int return_code_t;
#endif

class Exception : public std::exception {
private:
  std::string _message;
  return_code_t _err_number;

public:
  Exception(std::string message, return_code_t err_number)
      : _message(message + " Error number is " + std::to_string(err_number) +
                 "."),
        _err_number(err_number) {}

  const char *what() const noexcept override { return _message.c_str(); }

  const return_code_t get_err_number() const noexcept { return _err_number; }
};

} // namespace my::common
