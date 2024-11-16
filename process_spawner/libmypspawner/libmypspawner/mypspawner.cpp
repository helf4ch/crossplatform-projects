#include "mypspawner.hpp"
#include <csignal>
#include <cstddef>
#include <cstring>
#include <sys/wait.h>
#include <spawn.h>

class my::PSpawner::PSpawnerImpl {
public:
  int pid;

  std::string path;

  std::string pname;

  std::vector<std::string> argv;

  std::vector<std::string> envp;

  bool is_running;
};

my::PSpawner::PSpawner(const std::string path, const std::string pname,
                       const std::vector<std::string> argv,
                       const std::vector<std::string> envp) {
  spawner = new PSpawnerImpl;
  spawner->pid = 0;
  spawner->path = path;
  spawner->pname = pname;
  spawner->argv = argv;
  spawner->argv.insert(spawner->argv.begin(), spawner->pname);
  spawner->argv.push_back("\0");
  spawner->envp = envp;
  spawner->envp.push_back("\0");
  spawner->is_running = false;
}

my::PSpawner::~PSpawner() { delete spawner; }

char **to_c_style_str_list(const std::vector<std::string> &v) {
  char **res = new char *[v.size()];
  for (auto i = 0; i < v.size(); ++i) {
    res[i] = new char[v[i].size() + 1];
    strcpy(res[i], v[i].c_str());
  }
  return res;
}

void free_c_style_str_list(char ***list, size_t n) {
  for (auto i = 0; i < n; ++i) {
    delete (*list)[i];
  }
  delete *list;
}

// TODO: add exception for any error
void my::PSpawner::start() {
  char **argv = to_c_style_str_list(get_argv());
  char **envp = to_c_style_str_list(get_envp());

  auto status = posix_spawn(&this->spawner->pid, get_path().c_str(), NULL, NULL,
                            argv, envp);

  free_c_style_str_list(&argv, get_argv().size());
  free_c_style_str_list(&envp, get_envp().size());
}

bool my::PSpawner::is_running() {
  auto res = kill(spawner->pid, 0);
  if (!res) {
    spawner->is_running = true;
  } else {
    spawner->is_running = false;
  }
  return spawner->is_running;
}

// TODO: add exception for any error
void my::PSpawner::wait() {
  if (!spawner->is_running) {
    return;
  }
  int status;
  int res = waitpid(spawner->pid, &status, 0);
  if (res != spawner->pid) {
    return;
  }
}

int my::PSpawner::get_pid() const noexcept { return spawner->pid; }

const std::string &my::PSpawner::get_path() const noexcept {
  return spawner->path;
}

const std::string &my::PSpawner::get_pname() const noexcept {
  return spawner->pname;
}

const std::vector<std::string> &my::PSpawner::get_argv() const noexcept {
  return spawner->argv;
}

const std::vector<std::string> &my::PSpawner::get_envp() const noexcept {
  return spawner->envp;
}
