#pragma once

#include <string>

namespace my {

class Semaphore {
public:
  Semaphore(const std::string &name = "");

  ~Semaphore();

  void post() const;

  void wait() const;
  
private:
  class SemImpl;

  SemImpl *sem;
};

} // namespace my
