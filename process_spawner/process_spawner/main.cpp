#include "libmypspawner/mypspawner.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

int main(int argc, char **argv) {
  if (argc == 1) {
    std::cout << "DESCRIPTION: Process spawner lib example.\n";
    std::cout
        << "USAGE: <filepath> [--argv <...> <...>] [--envp <...> <...>]\n";

    return 0;
  }

  std::vector<std::string> args_list(argv + 1, argv + argc);

  std::string opt_filepath = args_list[0];
  // std::string opt_filepath = "C:\\Users\\kiril\\projects\\crossplatform-projects\\process_spawner\\files\\repeat_hello.exe";

  std::vector<std::string> opt_argv = {};
  std::vector<std::string> opt_envp = {};

  auto opt_argv_it =
      std::find(args_list.begin() + 1, args_list.end(), "--argv");
  auto opt_envp_it =
      std::find(args_list.begin() + 1, args_list.end(), "--envp");

  if (opt_argv_it != args_list.end()) {
    if (opt_argv_it < opt_envp_it) {
      opt_argv.insert(opt_argv.end(), opt_argv_it + 1, opt_envp_it);
    } else {
      opt_argv.insert(opt_argv.end(), opt_argv_it + 1, args_list.end());
    }
  }

  if (opt_envp_it != args_list.end()) {
    if (opt_argv_it < opt_envp_it) {
      opt_envp.insert(opt_envp.end(), opt_envp_it + 1, args_list.end());
    } else {
      opt_envp.insert(opt_envp.end(), opt_envp_it + 1, opt_argv_it);
    }
  }

  std::chrono::milliseconds await_time(5000);
  std::chrono::milliseconds check_time(1000);

  my::PSpawner spawner(opt_filepath, opt_argv, opt_envp);

  spawner.start();

  std::cout << "[root] child pid: " << spawner.get_pid() << '\n';

  for (int i = 0; i < await_time / check_time; ++i) {
    std::cout << "[root] is_running: " << spawner.is_running() << '\n';
    std::this_thread::sleep_for(check_time);
  }

  std::cout << "[root] starting wait...\n";

  auto status = spawner.wait();

  std::cout << "[root] wait is done\n";
  std::cout << "[root] is_running: " << spawner.is_running() << '\n';
  std::cout << "[root] return code: " << status << '\n';

  return 0;
}
