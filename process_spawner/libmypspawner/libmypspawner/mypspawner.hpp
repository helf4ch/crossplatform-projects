#pragma once

#include <string>
#include <vector>

namespace my {

class PSpawner {
public:
  PSpawner(const std::string path, const std::string pname,
          const std::vector<std::string> argv,
          const std::vector<std::string> envp);

  ~PSpawner();

  void start();

  bool is_running();

  int wait();

  void kill();

  int get_pid() const noexcept;

  const std::string &get_path() const noexcept;

  const std::string &get_pname() const noexcept;

  const std::vector<std::string> &get_argv() const noexcept;

  const std::vector<std::string> &get_envp() const noexcept;

private:
  class PSpawnerImpl;
  PSpawnerImpl *spawner;
};

} // namespace my
