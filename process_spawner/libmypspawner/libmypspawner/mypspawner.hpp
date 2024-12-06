#pragma once

#include <exception>
#include <string>
#include <vector>

namespace my {

class PSpawner {
public:
  PSpawner(const std::string path, const std::vector<std::string> argv,
           const std::vector<std::string> envp);

  ~PSpawner();

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
                   "."), _err_number(err_number) {}

    const char *what() const noexcept override { return _message.c_str(); }

    const return_code_t get_err_number() const noexcept {
      return _err_number;
    }
  };

  pid_t start();

  bool is_running();

  return_code_t wait();

  void kill();

  const pid_t get_pid() const noexcept;

  const std::string &get_path() const noexcept;

  const std::vector<std::string> &get_argv() const noexcept;

  const std::vector<std::string> &get_envp() const noexcept;

private:
  class PSpawnerImpl;

  PSpawnerImpl *spawner;
};

} // namespace my
