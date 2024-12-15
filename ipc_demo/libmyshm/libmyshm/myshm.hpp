#pragma once

#include <cstddef>
#include <string>

namespace my {

template <typename T>
class SharedMemory {
public:
  SharedMemory(const std::string &name, std::size_t size = sizeof(T));

  ~SharedMemory();

  void *getRaw() const noexcept;  

  T *getTyped() const noexcept;  

  T *operator->() const noexcept;

private:
  template <typename S>
  class ShmImpl;

  ShmImpl<T> *shm;
};

} // namespace my

#include "myshm.tpp"
