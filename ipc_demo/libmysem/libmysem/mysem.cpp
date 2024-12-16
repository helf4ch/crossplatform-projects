#include "mysem.hpp"

#include "libmyshm/myshm.hpp"
#include "libmycommon/mycommon.hpp"
#include <memory>

#ifdef _WIN32
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <semaphore.h>
#endif

#ifdef _WIN32
#define MY_SEMAPHORE_NAME_PREPEND "Global\\my-semaphore-"
#else
#define MY_SEMAPHORE_NAME_PREPEND "my-semaphore-"
#endif

class my::Semaphore::SemImpl {
public:
#ifdef _WIN32
  void *sem_p;
#else
  sem_t *sem_p;
#endif
  std::string sem_name;
  std::unique_ptr<my::SharedMemory<int>> sem_counter;
};

my::Semaphore::Semaphore(const std::string &name) {
  sem = new SemImpl;

  sem->sem_name = MY_SEMAPHORE_NAME_PREPEND + name;

#ifdef _WIN32
  sem->sem_p = CreateSemaphore(NULL, 0, 32000, sem->sem_name.c_str());
  if (!sem->sem_p) {
    throw my::common::Exception("Error in my::Semaphore::Semaphore (CreateSemaphoreA).", GetLastError());
  }
  post();
#else
  sem->sem_p = sem_open(sem->sem_name.c_str(), O_CREAT, 0777, 0);
  if (sem->sem_p == SEM_FAILED) {
    throw my::common::Exception("Error in Semaphore::Semaphore. (sem_open)", errno);
  }
  post();
  
  sem->sem_counter = std::make_unique<my::SharedMemory<int>>(
      MY_SEMAPHORE_NAME_PREPEND + std::string{"private-obj"});

  *sem->sem_counter->getTyped() += 1;
#endif
}

my::Semaphore::~Semaphore() {
#ifdef _WIN32
  CloseHandle(sem->sem_p);
#else
  sem_close(sem->sem_p);

  *sem->sem_counter->getTyped() -= 1;
  if (*sem->sem_counter->getTyped() == 0) {
    sem_unlink(sem->sem_name.c_str());
  }
#endif

  delete sem;
}

void my::Semaphore::post() const {
#ifdef _WIN32
  bool result = ReleaseSemaphore(sem->sem_p, 1, NULL);
  if (!result) {
    throw my::common::Exception("Error in Semaphore::post.", GetLastError());
  }
#else
  int result = sem_post(sem->sem_p);
  if (result) {
    throw my::common::Exception("Error in Semaphore::post.", errno);
  }
#endif
}

void my::Semaphore::wait() const {
#ifdef _WIN32
  DWORD result = WaitForSingleObject(sem->sem_p, 0);
  if (result == WAIT_FAILED) {
    throw my::common::Exception("Error in Semaphore::wait.", GetLastError());
  }
#else
  int result = sem_wait(sem->sem_p);
  if (result) {
    throw my::common::Exception("Error in Semaphore::wait.", errno);
  }
#endif
}
