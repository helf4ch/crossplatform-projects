// #include <cerrno>
// #include <sys/types.h>
// #include <unistd.h>
// #include <sys/stat.h>
// #include <fcntl.h>

#include <nlohmann/json.hpp>
#include <iostream>

int main() {
  // auto d = open("/dev/pts/4", O_WRONLY);

  // if (d == 0) {
  //   std::cout << "error " << errno;
  // }

  // char buf[] = "hello\n";
  // write(d, buf, 10);

  std::string str = R"(
    {
      "temp": {
        "now": 299.2,
        "future": 299.3
      },
      "feels_like": 262.5
    }
  )";

  nlohmann::json json = nlohmann::json::parse(str);

  std::cout << json["temp"]["future"];

  return 0;
}
