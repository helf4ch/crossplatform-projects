#include <exception>
#include <string>

namespace my::common {

class Exception : public std::exception {
private:
  std::string _message;
  int _err_number;

public:
  Exception(std::string message, int err_number)
      : _message(message + " Error number is " + std::to_string(err_number) +
                 "."),
        _err_number(err_number) {}

  const char *what() const noexcept override { return _message.c_str(); }

  const int get_err_number() const noexcept { return _err_number; }
};

} // namespace my::common
