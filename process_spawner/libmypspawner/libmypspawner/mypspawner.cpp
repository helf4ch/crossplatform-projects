#include "mypspawner.hpp"

#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstring>
#include <string>

#ifdef WIN32
#include <sstream>
#include <windows.h>
#else
#include <spawn.h>
#include <sys/wait.h>
#endif

class my::PSpawner::PSpawnerImpl {
public:
  my::PSpawner::pid_t pid;

  std::string path;

  std::vector<std::string> argv;

  std::vector<std::string> envp;

  bool is_running;

#ifdef WIN32
  HANDLE h_process;
#endif
};

my::PSpawner::PSpawner(const std::string path,
                       const std::vector<std::string> argv,
                       const std::vector<std::string> envp) {
  spawner = new PSpawnerImpl;
  spawner->pid = 0;
  spawner->path = path;
  spawner->argv = argv;
  spawner->argv.insert(spawner->argv.begin(), spawner->path);
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
    delete[] (*list)[i];
  }
  delete[] *list;
}

// TODO: add exception for any error
my::PSpawner::pid_t my::PSpawner::start() {
  char **envp = to_c_style_str_list(get_envp());

#ifdef WIN32
  std::stringstream strstream;
  for (size_t i = 0; i < get_argv().size() - 1; ++i) {
    strstream << get_argv()[i] << ' ';
  }
  strstream << get_argv()[get_argv().size() - 1];

  auto str = strstream.str();
  char *argv = new char[str.size() + 1];
  strcpy(argv, str.c_str());

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  CreateProcess(NULL, argv, NULL, NULL, FALSE, 0, envp, NULL, &si, &pi);

  spawner->h_process = pi.hProcess;
  spawner->pid = pi.dwProcessId;

  delete[] argv;
#else
  char **argv = to_c_style_str_list(get_argv());

  int result = posix_spawn(&this->spawner->pid, get_path().c_str(), NULL, NULL,
                           argv, envp);

  if (result) {
    throw Exception("Error in PSpawner::start.", errno);
  }

  free_c_style_str_list(&argv, get_argv().size());
#endif

  free_c_style_str_list(&envp, get_envp().size());

  return spawner->pid;
}

bool my::PSpawner::is_running() {
#ifdef WIN32
  retunr_code_t res;
  GetExitCodeProcess(spawner->h_process, &res);
#else
  return_code_t result = ::kill(spawner->pid, 0);

  if (!result && errno == ESRCH) {
    spawner->is_running = false;
  } else {
    spawner->is_running = true;
  }
#endif

  return spawner->is_running;
}

// TODO: add exception for any error
my::PSpawner::return_code_t my::PSpawner::wait() {
#ifdef WIN32
  WaitForSingleObject(spawner->h_process, INFINITE);

  return_code_t status;
  GetExitCodeProcess(spawner->h_process, &status);
#else
  return_code_t status;
  int result = waitpid(spawner->pid, &status, 0);
  
  if (result != spawner->pid) {
    throw Exception("Error in PSpawner::wait.", errno);
  }
#endif

  return status;
}

void my::PSpawner::kill() {
#ifdef WIN32
  TerminateProcess(spawner->h_process, 1);
#else
  int result = ::kill(spawner->pid, SIGTERM);

  if (!result && errno == EPERM) {
    throw Exception("Error in PSpawner::kill.", errno);
  }
#endif
}

my::PSpawner::pid_t my::PSpawner::get_pid() const noexcept {
  return spawner->pid;
}

const std::string &my::PSpawner::get_path() const noexcept {
  return spawner->path;
}

const std::vector<std::string> &my::PSpawner::get_argv() const noexcept {
  return spawner->argv;
}

const std::vector<std::string> &my::PSpawner::get_envp() const noexcept {
  return spawner->envp;
}
