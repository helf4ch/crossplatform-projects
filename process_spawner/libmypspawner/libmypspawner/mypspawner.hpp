#pragma once

#include <string>
#include <vector>

namespace my {

class PSpawner {
public:
  PSpawner(const std::string path, const std::vector<std::string> argv,
           const std::vector<std::string> envp);

  ~PSpawner();

#ifdef WIN32

  typedef unsigned long pid_t;
  typedef unsigned long return_code_t;

#else

  typedef int pid_t;
  typedef int return_code_t;

#endif

  pid_t start();

  bool is_running();

  return_code_t wait();

  void kill();

  pid_t get_pid() const noexcept;

  const std::string &get_path() const noexcept;

  const std::vector<std::string> &get_argv() const noexcept;

  const std::vector<std::string> &get_envp() const noexcept;

private:
  class PSpawnerImpl;
  PSpawnerImpl *spawner;
};

} // namespace my
