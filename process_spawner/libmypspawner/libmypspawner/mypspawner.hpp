#pragma once

#include <string>
#include <vector>

namespace my {

class PSpawner {
public:
  PSpawner(const std::string path, const std::vector<std::string> argv,
           const std::vector<std::string> envp);

  ~PSpawner();

#ifdef _WIN32

  typedef DWORD pid_t;
  typedef DWORD return_code_t;

#else

  typedef int pid_t;
  typedef int return_code_t;

#endif

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
