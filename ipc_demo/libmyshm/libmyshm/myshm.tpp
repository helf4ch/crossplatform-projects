#pragma once

#include "libmycommon/mycommon.hpp"
#include "myshm.hpp"
#include <unistd.h>
#include <cstddef>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif

#ifdef _WIN32
#define MY_SHAREDMEMORY_NAME_PREPEND "Global\\my-sharedmemory-"
#else
#define MY_SHAREDMEMORY_NAME_PREPEND "my-sharedmemory-"
#endif

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

#ifdef _WIN32
  shm->block_size = size;
  
  void *shm_h = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, shm->name.c_str());
  if (!shm_h) {
    throw my::common::Exception("Error in my::SharedMymory::SharedMemory (CreateFileMapping).", GetLastError()); 
  }

  shm->addr = MapViewOfFile(shm_h, FILE_MAP_ALL_ACCESS, 0, 0, shm->block_size);
  memset(shm->addr, 0, shm->block_size);
  
  shm->data_raw_p = shm->addr;

  CloseHandle(shm_h);
#else
  int shm_d = shm_open(shm->name.c_str(), O_CREAT | O_RDWR, 0777);
  if (shm_d == -1) {
    throw my::common::Exception(
        "Error in my::SharedMemory::SharedMemory (shm_open).", errno);
  }

  shm->block_size = size + sizeof(ShmHeader);
  int ftrunc_res = ftruncate(shm_d, shm->block_size);
  if (ftrunc_res == -1) {
    throw my::common::Exception(
        "Error in my::SharedMemory::SharedMemory (ftruncate).", errno);
  }

  shm->addr = mmap(NULL, shm->block_size, PROT_EXEC | PROT_READ | PROT_WRITE,
                   MAP_SHARED, shm_d, 0);
  if (shm->addr == (void *)-1) {
    throw my::common::Exception(
        "Error in my::SharedMemory::SharedMemory (mmap).", errno);
  }

  close(shm_d);
  
  shm->header_p = static_cast<ShmHeader *>(shm->addr);
  shm->header_p->use_count += 1;
  
  shm->data_raw_p = shm->header_p + 1;
#endif
  shm->data_typed_p = static_cast<T *>(shm->data_raw_p);
}

template <typename T> my::SharedMemory<T>::~SharedMemory() {
#ifdef _WIN32
  UnmapViewOfFile(shm->addr);
#else
  shm->header_p->use_count -= 1;

  if (shm->header_p->use_count == 0) {
    munmap(shm->addr, shm->block_size);
    shm_unlink(shm->name.c_str());
  } else {
    munmap(shm->addr, shm->block_size);
  }
#endif

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
