#include "libmysem/mysem.hpp"
#include "libmyshm/myshm.hpp"
#include <algorithm>
#include <csetjmp>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
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

class IPLog {
public:
  IPLog(const std::string &file_name) {
    sem = std::make_unique<my::Semaphore>(file_name);
    sem->wait();

    shm_buf = std::make_unique<my::SharedMemory<buf>>(file_name);

    if ((*shm_buf)->use_count == 0) {
      is_file_owner = true;
      is_continue_handling = true;
      file = fopen(file_name.c_str(), "w+");
    }

    (*shm_buf)->use_count += 1;

    sem->post();
  }

  ~IPLog() {
    if (is_file_owner) {
      fclose(file);
    }
  }

  void write() {
    lock();
    std::cout << str_stream.str();

    strcpy(get_shm()->msg, str_stream.str().c_str());
    get_shm()->msg_count += 1;

    str_stream.str("");

    unlock();
  }

  void start_handling() {
    if (!is_file_owner) {
      return;
    }

    handle_thread = std::thread(&IPLog::handling, this);
  }

  void stop_handling() {
    if (!is_file_owner) {
      return;
    }

    is_continue_handling = false;
    handle_thread.join();
  }

  friend std::stringstream &operator<<(IPLog &obj,
                                       const std::string &str);

private:
  const static uint32_t BUF_SIZE = 255;

  struct buf {
    int use_count;
    int msg_count;
    char msg[BUF_SIZE];
  };

  std::unique_ptr<my::Semaphore> sem;
  std::unique_ptr<my::SharedMemory<buf>> shm_buf;

  bool is_file_owner = false;
  FILE *file = NULL;

  std::stringstream str_stream;

  bool is_continue_handling;
  std::thread handle_thread;

  const my::SharedMemory<buf> &get_shm() { return *shm_buf; }

  const my::Semaphore &get_sem() { return *sem; }

  void lock() { get_sem().wait(); }

  void unlock() { get_sem().post(); }

  void handling() {
    while (is_continue_handling) {
      if (get_shm()->msg_count > 0) {
        fprintf(file, "%s", get_shm()->msg);
        get_shm()->msg_count -= 1;
      }
    }
  }
};

std::stringstream &operator<<(IPLog &obj, const std::string &str) {
  obj.str_stream << str;
  return obj.str_stream;
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
              IPLog &log) {
  bool is_exit = false;
  while (!is_exit) {
    try {
      sem.wait();

      log << "[" << get_ctime_string() << "] PID " << get_current_pid() << " posted "
           << shm->counter << '\n';
      log.write();

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
  IPLog log("log.txt");

  my::SharedMemory<Data> shm("myshm");

  my::Semaphore sem("mysem");

  log.start_handling();

  log << "[" << get_ctime_string() << "] started in PID " << get_current_pid()
      << '\n';

  log.write();

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

  log.stop_handling();

  return 0;
}
