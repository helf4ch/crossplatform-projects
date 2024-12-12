#include "libmyshm/myshm.hpp"
#include "libmysem/mysem.hpp"
#include <iostream>
#include <thread>
#include <unistd.h>

struct Test {
  int num;
};

void myfunc1(my::SharedMemory<Test> *shm, my::Semaphore *sem) {
  std::cout << "func1 " << (*shm)->num << '\n';
  sem->post();
  sleep(1);
  sem->wait();
  std::cout << "func1 " << (*shm)->num << '\n';
}

void myfunc2(my::SharedMemory<Test> *shm, my::Semaphore *sem) {
  sem->wait();
  (*shm)->num += 1;
  sem->post();
}

int main(int argc, char **argv) {
  my::SharedMemory<Test> shm("name1");
  my::Semaphore sem("name9");

  std::thread f1(myfunc1, &shm, &sem);
  std::thread f2(myfunc2, &shm, &sem);
  
  f1.join();
  f2.join();

  return 0;
}
