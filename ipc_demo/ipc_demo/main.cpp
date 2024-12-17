#include "libmysem/mysem.hpp"
#include "libmyshm/myshm.hpp"
#include <algorithm>
#include <chrono>
#include <csetjmp>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

jmp_buf env;

void signalHandler(int signum) { longjmp(env, signum); }

int get_current_pid() { return getpid(); }

std::string get_ctime_string() {
  std::stringstream buffer;
  std::time_t t = std::time(0);
  std::tm *now = std::localtime(&t);
  buffer << std::put_time(now, "%d.%m.%Y %H:%M:%S");
  return buffer.str();
}

bool getline_nonblocking(std::istream &in, std::string &str) {
  char ch;
  int count = in.readsome(&ch, 1);

  bool line_end = false;
  if (count) {
    if (ch == '\n') {
      line_end = true;
    } else {
      str.append(1, ch);
    }
  }

  return line_end;
}

struct Data {
  bool exit_flag;
  int use_count;
  int counter;
};

void increase_300ms(const my::SharedMemory<Data> &shm, const my::Semaphore &sem,
                    bool &is_exit) {
  while (!is_exit) {
    try {
      sem.wait();
      shm->counter += 1;
      sem.post();

      std::this_thread::sleep_for(std::chrono::milliseconds(300));
    } catch (std::exception &e) {
      std::cerr << e.what();
      is_exit = true;
    }
  }
}

void write_1s(const my::SharedMemory<Data> &shm, const my::Semaphore &sem,
              std::fstream &log, bool &is_exit) {
  while (!is_exit) {
    try {
      sem.wait();

      log << "[" << get_ctime_string() << "] PID " << get_current_pid()
          << " posted " << shm->counter << '\n';

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

  sem.wait();
  bool is_main_process = false;
  if (shm->use_count == 0) {
    is_main_process = true;
  }
  shm->use_count += 1;
  sem.post();

  bool is_exit = false;
  std::thread th_increase_300ms(increase_300ms, std::cref(shm), std::cref(sem),
                                std::ref(is_exit));
  std::thread th_write_1s;
  if (is_main_process) {
    th_write_1s = std::thread(write_1s, std::cref(shm), std::cref(sem),
                              std::ref(log), std::ref(is_exit));
  }

  std::cout << "Commands: set; get; exit.\n";

  std::string input;
  std::ios_base::sync_with_stdio(false);

  std::signal(SIGABRT, signalHandler);
  std::signal(SIGFPE, signalHandler);
  std::signal(SIGILL, signalHandler);
  std::signal(SIGINT, signalHandler);
  std::signal(SIGSEGV, signalHandler);
  std::signal(SIGTERM, signalHandler);

  int err = setjmp(env);

  if (err) {
    std::cout << "SIGNAL: " << err << '\n';
  }

  while (!err && !is_exit) {
    input.clear();

    while (!getline_nonblocking(std::cin, input)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      if (!shm->exit_flag) {
        continue;
      }

      if (!is_main_process) {
        std::cout << "Root called to exit.\n";
      }
      is_exit = true;
      break;
    }

    std::vector<std::string> str_splitted;

    std::string::iterator it;
    while ((it = std::find(input.begin(), input.end(), ' ')) != input.end()) {
      str_splitted.emplace_back(input.begin(), it);
      input.erase(input.begin(), it + 1);
    }
    str_splitted.push_back(input);

    if (str_splitted[0] == "get") {
      sem.wait();
      std::cout << "Counter is: " << shm->counter << '\n';
      sem.post();
    } else if (str_splitted[0] == "exit") {
      break;
    } else if (str_splitted[0] == "set") {
      if (str_splitted.size() == 1) {
        std::cout << "Usage: set <number>\n";
        continue;
      }
      try {
        int num = std::stoi(str_splitted[1]);
        sem.wait();
        shm->counter = num;
        sem.post();
      } catch (std::exception &e) {
        std::cout << "Usage: set <number>\n";
      }
    } else if (input.size() != 0) {
      std::cout << "Commands: set; get; exit.\n";
    }
  }

  std::cout << "Exiting...\n";

  is_exit = true;

  sem.wait();
  if (is_main_process) {
    shm->exit_flag = true;
  }
  shm->use_count -= 1;
  sem.post();

  th_increase_300ms.join();

  if (is_main_process) {
    th_write_1s.join();
  }

  std::cout << "Exited.\n";

  return 0;
}
