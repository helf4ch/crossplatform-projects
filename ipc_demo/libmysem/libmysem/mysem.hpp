#pragma once

#include <string>

namespace my {

class Semaphore {
public:
  Semaphore(const std::string &name);

  ~Semaphore();

  void post();

  void wait();
  
private:
  class SemImpl;

  SemImpl *sem;
};

} // namespace my
