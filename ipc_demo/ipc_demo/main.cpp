#include "libmyshm/myshm.hpp"
#include <iostream>

struct Test {
  int num;
};

int main(int argc, char **argv) {
  my::SharedMemory<Test> shm("name1");

  std::cout << shm->num << '\n';

  shm->num += 1;
  
  std::cout << shm->num << '\n';
  
  return 0;
}
