#include "libmypspawner/mypspawner.hpp"
#include <chrono>
#include <iostream>
#include <thread>

int main() {
  std::chrono::milliseconds await_time(5000);
  std::chrono::milliseconds check_time(1000);

  std::string filepath =
      "/home/helf4ch/Projects/OpSystems/process_spawner/files/repeat_hello.sh";
  std::string pname = "awesome_repeater";

  my::PSpawner spawner(filepath, pname, {}, {});

  spawner.start();

  std::cout << "[root] child pid: " << spawner.get_pid() << '\n';

  for (int i = 0; i < await_time / check_time; ++i) {
    std::cout << "[root] is_running: " << spawner.is_running() << '\n';
    std::this_thread::sleep_for(check_time);
  }

  std::cout << "[root] starting wait...\n";

  spawner.wait();

  std::cout << "[root] wait is done\n";
  std::cout << "[root] is_running: " << spawner.is_running() << '\n';

  return 0;
}
