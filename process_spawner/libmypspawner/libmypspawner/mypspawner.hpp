#pragma once

#include "libmycommon/mycommon.hpp"
#include <string>
#include <vector>

namespace my {

class PSpawner {
public:
  PSpawner(const std::string path, const std::vector<std::string> argv,
           const std::vector<std::string> envp);

  ~PSpawner();

  pid_t start();

  bool is_running();

  my::common::return_code_t wait();

  void kill();

  const my::common::pid_t get_pid() const noexcept;

  const std::string &get_path() const noexcept;

  const std::vector<std::string> &get_argv() const noexcept;

  const std::vector<std::string> &get_envp() const noexcept;

private:
  class PSpawnerImpl;

  PSpawnerImpl *spawner;
};

} // namespace my
