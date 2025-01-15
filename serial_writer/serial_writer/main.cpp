// #include <cerrno>
// #include <sys/types.h>
// #include <unistd.h>
// #include <sys/stat.h>
// #include <fcntl.h>

#include "libmyhttp/myhttp.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

int main() {
  // auto d = open("/dev/pts/4", O_WRONLY);

  // if (d == 0) {
  //   std::cout << "error " << errno;
  // }

  // char buf[] = "hello\n";
  // write(d, buf, 10);

  nlohmann::json ex = nlohmann::json::parse(R"(
    {
      "pi": 3.141,
      "happy": true
    }
  )");

  std::string dump = ex.dump();

  std::string req = "GET /url?da=jopa HTTP/1.1\r\n"
                    "Host: jopa.com\r\n"
                    "\r\n";

  my::http::Request::parse(req);

  return 0;
}
