#include "libmysem/mysem.hpp"
#include "libmyshm/myshm.hpp"
#include <algorithm>
#include <csetjmp>
#include <csignal>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

jmp_buf env;

void signalHandler(int signum) { longjmp(env, 1); }

int get_current_pid() { return getpid(); }

std::string get_ctime_string() {
  std::stringstream buffer;
  std::time_t t = std::time(0); // get time now
  std::tm *now = std::localtime(&t);
  buffer << std::put_time(now, "%d.%m.%Y %H:%M:%S");
  return buffer.str();
}

struct Data {
  bool exit_flag;
  int counter;
};

void increase_300ms(const my::SharedMemory<Data> &shm,
                    const my::Semaphore &sem) {
  bool is_exit = false;
  while (!is_exit) {
    try {
      sem.wait();
      shm->counter += 1;
      is_exit = shm->exit_flag;
      sem.post();
      std::this_thread::sleep_for(std::chrono::milliseconds(300));
    } catch (std::exception &e) {
      std::cerr << e.what();
      is_exit = true;
    }
  }
}

void write_1s(const my::SharedMemory<Data> &shm, const my::Semaphore &sem,
              std::fstream &file) {
  bool is_exit = false;
  while (!is_exit) {
    try {
      sem.wait();
      file << "[" << get_ctime_string() << "] PID " << get_current_pid() << ": "
           << shm->counter << '\n';
      is_exit = shm->exit_flag;
      sem.post();
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    } catch (std::exception &e) {
      std::cerr << e.what();
      is_exit = true;
    }
  }
}

int main(int argc, char **argv) {
  std::fstream log("log.txt", std::fstream::out);

  my::SharedMemory<Data> shm("myshm");

  my::Semaphore sem("mysem");

  log << "[" << get_ctime_string() << "] started in PID " << get_current_pid()
      << '\n';

  std::thread th_increase_300ms(increase_300ms, std::cref(shm), std::cref(sem));
  std::thread th_write_1s(write_1s, std::cref(shm), std::cref(sem),
                          std::ref(log));

  std::cout << "Commands: set; get; exit.\n";

  std::string input;

  std::signal(SIGABRT, signalHandler);
  std::signal(SIGFPE, signalHandler);
  std::signal(SIGILL, signalHandler);
  std::signal(SIGINT, signalHandler);
  std::signal(SIGSEGV, signalHandler);
  std::signal(SIGTERM, signalHandler);

  int err = setjmp(env);

  while (getline(std::cin, input) && !err) {
    std::vector<std::string> str_splitted;

    std::string::iterator it;
    while ((it = std::find(input.begin(), input.end(), ' ')) != input.end()) {
      str_splitted.emplace_back(input.begin(), it);
      input.erase(input.begin(), it + 1);
    }
    str_splitted.push_back(input);

    if (str_splitted[0] == "get") {
      sem.wait();
      std::cout << "exit is " << shm->exit_flag << '\n';
      std::cout << "Counter is: " << shm->counter << '\n';
      sem.post();
    } else if (str_splitted[0] == "exit") {
      break;
    } else if (str_splitted[0] == "set") {
      if (str_splitted.size() == 1) {
        std::cout << "Usage: set <number>\n";
      }
      try {
        int num = std::stoi(str_splitted[1]);
        sem.wait();
        shm->counter = num;
        sem.post();
      } catch (std::exception &e) {
        std::cout << "Usage: set <number>\n";
      }
    } else {
      std::cout << "Commands: set; get; exit.\n";
    }
  }

  std::cout << "Exiting...\n";

  sem.wait();
  shm->exit_flag = true;
  sem.post();

  th_increase_300ms.join();
  th_write_1s.join();

  std::cout << "Exited.\n";

  log.close();

  return 0;
}
