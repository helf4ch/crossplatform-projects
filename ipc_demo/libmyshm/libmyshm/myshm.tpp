#pragma once 

#include "myshm.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

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

  shm->name = name;

  // FIX: error generation
  // int is_err = shm_open(name.c_str(), O_CREAT | O_EXCL, 0777);
  // bool is_exist = is_err == -1 && errno == EEXIST ? true : false;

  // FIX: error generation
  int shm_d = shm_open(name.c_str(), O_CREAT | O_RDWR, 0777);

  shm->block_size = size + sizeof(ShmHeader);
  // FIX: error generation
  ftruncate(shm_d, shm->block_size);
  
  // FIX: error generation
  shm->addr =
      mmap(NULL, shm->block_size, PROT_EXEC | PROT_READ | PROT_WRITE,
           MAP_SHARED, shm_d, 0);

  close(shm_d);

  shm->header_p = static_cast<ShmHeader *>(shm->addr);

  shm->data_raw_p = shm->header_p + 1;
  shm->data_typed_p = static_cast<T *>(shm->data_raw_p);

  shm->header_p->use_count += 1;
}

template <typename T> my::SharedMemory<T>::~SharedMemory() { 
  // FIX: error generation
  shm->header_p->use_count -= 1;

  if (shm->header_p->use_count == 0) {
  // FIX: error generation
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

