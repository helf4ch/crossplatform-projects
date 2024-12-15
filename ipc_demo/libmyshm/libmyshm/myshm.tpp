#pragma once 

#include "myshm.hpp"
#include "libmycommon/mycommon.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define MY_SHAREDMEMORY_NAME_PREPEND "my-sharedmemory-"

struct ShmHeader {
  uint32_t use_count;
};

template <typename T> template <typename S> class my::SharedMemory<T>::ShmImpl {
public:
  std::string name;

  std::size_t block_size;
  void *addr;

  ShmHeader *header_p;
  void *data_raw_p;
  T *data_typed_p;
};

template <typename T>
my::SharedMemory<T>::SharedMemory(const std::string &name, size_t size) {
  shm = new ShmImpl<T>;

  shm->name = MY_SHAREDMEMORY_NAME_PREPEND + name;

  int shm_d = shm_open(shm->name.c_str(), O_CREAT | O_RDWR, 0777);
  if (shm_d == -1) {
    throw my::common::Exception("Error in my::SharedMemory::SharedMemory (shm_open).", errno);
  }

  shm->block_size = size + sizeof(ShmHeader);
  int ftrunc_res = ftruncate(shm_d, shm->block_size);
  if (ftrunc_res == -1) {
    throw my::common::Exception("Error in my::SharedMemory::SharedMemory (ftruncate).", errno);
  }
  
  shm->addr =
      mmap(NULL, shm->block_size, PROT_EXEC | PROT_READ | PROT_WRITE,
           MAP_SHARED, shm_d, 0);
  if (shm->addr == (void *)-1) {
    throw my::common::Exception("Error in my::SharedMemory::SharedMemory (mmap).", errno);
  }

  close(shm_d);

  shm->header_p = static_cast<ShmHeader *>(shm->addr);

  shm->data_raw_p = shm->header_p + 1;
  shm->data_typed_p = static_cast<T *>(shm->data_raw_p);

  shm->header_p->use_count += 1;
}

template <typename T> my::SharedMemory<T>::~SharedMemory() { 
  shm->header_p->use_count -= 1;

  if (shm->header_p->use_count == 0) {
    munmap(shm->addr, shm->block_size);
    shm_unlink(shm->name.c_str());
  } else {
    munmap(shm->addr, shm->block_size);
  }

  delete shm; 
}

template <typename T> void *my::SharedMemory<T>::getRaw() const noexcept {
  return shm->data_raw_p;
}

template <typename T> T *my::SharedMemory<T>::getTyped() const noexcept {
  return shm->data_typed_p;
}

template <typename T> T *my::SharedMemory<T>::operator->() const noexcept {
  return getTyped();
}

