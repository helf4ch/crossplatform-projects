#include "mypspawner.hpp"

#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstring>
#include <string>

#ifdef _WIN32
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

#ifdef _WIN32
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
  spawner->envp = envp;
  spawner->is_running = false;
}

my::PSpawner::~PSpawner() { delete spawner; }

#ifndef _WIN32
char **to_c_style_str_list(const std::vector<std::string> &v) {
  size_t size = v.size() + 1;
  char **res = new char *[size];

  for (auto i = 0; i < v.size(); ++i) {
    res[i] = new char[v[i].size()];
    strcpy(res[i], v[i].c_str());
  }

  res[size - 1] = NULL;

  return res;
}

void free_c_style_str_list(char ***list) {
  char **ptr = *list;

  for (; *ptr; ++ptr) {
    delete[] *ptr;
  }

  delete[] *list;
}
#endif

my::PSpawner::pid_t my::PSpawner::start() {
#ifdef _WIN32
  std::stringstream argv_strstream;
  for (size_t i = 0; i < get_argv().size(); ++i) {
    argv_strstream << get_argv()[i];
    if (i < get_argv().size() - 1) {
      argv_strstream << ' ';
    }
  }

  auto argv_str = argv_strstream.str();

  char *parent_envp_str = GetEnvironmentStrings();
  auto ptr = parent_envp_str;
  int count = 0;
  while (*ptr) {
    count += strlen(ptr) + 1;
    ptr += strlen(ptr) + 1;
  }
  ++count;

  std::stringstream envp_strstream = std::stringstream();
  for (size_t i = 0; i < get_envp().size(); ++i) {
    envp_strstream << get_envp()[i] << '\0';
  }

  char *envp_str = new char[count + envp_strstream.str().size() + 1];
  memcpy(envp_str, envp_strstream.str().c_str(), envp_strstream.str().size());
  memcpy(envp_str + envp_strstream.str().size() + 1, parent_envp_str, count);
  *(envp_str + count + envp_strstream.str().size()) = '\0';

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  int result =
      CreateProcess(get_path().c_str(), const_cast<LPSTR>(argv_str.c_str()),
                    NULL, NULL, FALSE, 0, envp_str, NULL, &si, &pi);

  if (!result) {
    throw Exception("Error in PSpawner::start.", GetLastError());
  }

  spawner->h_process = pi.hProcess;
  spawner->pid = pi.dwProcessId;

  delete[] envp_str;
#else
  std::vector<std::string> argv_v = get_argv();
  argv_v.insert(argv_v.begin(), get_path());
  // argv_v.push_back("\0");

  char **argv = to_c_style_str_list(argv_v);

  std::vector<std::string> envp_v = get_argv();
  // envp_v.push_back("\0");

  char **envp = to_c_style_str_list(envp_v);

  int result = posix_spawn(&this->spawner->pid, get_path().c_str(), NULL, NULL,
                           argv, envp);

  if (result) {
    throw Exception("Error in PSpawner::start.", errno);
  }

  free_c_style_str_list(&argv);
  free_c_style_str_list(&envp);
#endif

  return spawner->pid;
}

bool my::PSpawner::is_running() {
#ifdef _WIN32
  return_code_t status;
  int result = GetExitCodeProcess(spawner->h_process, &status);

  if (!result) {
    throw Exception("Error in PSpawner::is_running.", GetLastError());
  }

  if (status == STILL_ACTIVE) {
    spawner->is_running = true;
  } else {
    spawner->is_running = false;
  }
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

my::PSpawner::return_code_t my::PSpawner::wait() {
#ifdef _WIN32
  WaitForSingleObject(spawner->h_process, INFINITE);

  return_code_t status;
  int result = GetExitCodeProcess(spawner->h_process, &status);

  if (result && GetLastError() != ERROR_SUCCESS) {
    throw Exception("Error in PSpawner::wait.", GetLastError());
  }
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
#ifdef _WIN32
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
