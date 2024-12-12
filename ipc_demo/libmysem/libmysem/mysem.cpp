#include "mysem.hpp"
#include <cerrno>
#include <semaphore.h>
#include <fcntl.h> 
#include <iostream>

class my::Semaphore::SemImpl {
public:
  sem_t *sem_p;  
};

my::Semaphore::Semaphore(const std::string &name) {
  sem = new SemImpl;

  sem->sem_p = sem_open(name.c_str(), O_CREAT, 0777, 0);
  if (sem->sem_p == SEM_FAILED) {
    //TODO: error
    std::cout << "error in open " << errno << '\n';
  }
}

my::Semaphore::~Semaphore() {
  sem_close(sem->sem_p);
  delete sem;  
}

void my::Semaphore::post() {
  int result = sem_post(sem->sem_p);
  if (result) {
    //TODO: error
    std::cout << "error in post " << errno << '\n';
  }
}

void my::Semaphore::wait() {
  int result = sem_wait(sem->sem_p);
  if (result) {
    //TODO: error
    std::cout << "error in wait " << errno << '\n';
  }
}
