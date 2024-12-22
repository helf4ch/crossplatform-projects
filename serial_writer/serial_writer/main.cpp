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

  using namespace my::http;
  Request req = Request::gen(
      Request::request_t::GET, {"127.0.0.1"}, "/some/path",
      {{Header::header_t::Content_type, "Application/json"},
       {Header::header_t::Content_lenght, std::to_string(dump.size())}},
      {{"some", "param"}, {"some1", "param1"}}, dump.c_str(), dump.size());

  std::cout << req.get_str();

  return 0;
}
