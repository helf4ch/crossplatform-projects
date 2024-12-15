#include "mysem.hpp"

#include "libmyshm/myshm.hpp"
#include "libmycommon/mycommon.hpp"
#include <cerrno>
#include <fcntl.h>
#include <memory>
#include <semaphore.h>

#define MY_SEMAPHORE_NAME_PREPEND "my-semaphore-"

class my::Semaphore::SemImpl {
public:
  sem_t *sem_p;
  std::string sem_name;
  std::unique_ptr<my::SharedMemory<int>> sem_counter;
};

my::Semaphore::Semaphore(const std::string &name) {
  sem = new SemImpl;

  sem->sem_name = MY_SEMAPHORE_NAME_PREPEND + name;

  sem->sem_p = sem_open(sem->sem_name.c_str(), O_CREAT, 0777, 0);
  if (sem->sem_p == SEM_FAILED) {
    throw my::common::Exception("Error in Semaphore::Semaphore. (sem_open)", errno);
  }

  sem->sem_counter = std::make_unique<my::SharedMemory<int>>(
      MY_SEMAPHORE_NAME_PREPEND + std::string{"private-obj"});

  *sem->sem_counter->getTyped() += 1;
}

my::Semaphore::~Semaphore() {
  sem_close(sem->sem_p);

  *sem->sem_counter->getTyped() -= 1;
  if (*sem->sem_counter->getTyped() == 0) {
    sem_unlink(sem->sem_name.c_str());
  }

  delete sem;
}

void my::Semaphore::post() const {
  int result = sem_post(sem->sem_p);
  if (result) {
    throw my::common::Exception("Error in Semaphore::post.", errno);
  }
}

void my::Semaphore::wait() const {
  int result = sem_wait(sem->sem_p);
  if (result) {
    throw my::common::Exception("Error in Semaphore::wait.", errno);
  }
}
