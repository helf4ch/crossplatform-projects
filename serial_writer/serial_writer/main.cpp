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

  auto r = my::http::Request::parse(req);

  std::cout << "type: " << r.get_type() << '\n';
  std::cout << "url: " << r.get_url() << '\n';
  std::cout << "param: " << r.get_param("da") << '\n';
  std::cout << "header: " << r.get_header("Host") << '\n';

  std::string res = "HTTP/1.1 201 Created\r\n"
                    "Content-Type: application/json\r\n"
                    "Location: http://example.com/users/123\r\n";

  std::cout << '\n';

  auto r1 = my::http::Response::parse(res);
  std::cout << "http: " << r1.get_http_ver() << '\n';
  std::cout << "code: " << r1.get_code() << '\n';
  std::cout << "text: " << r1.get_text() << '\n';
  std::cout << "header: " << r1.get_header("Content-Type") << '\n';


  std::string request = "GET /data/2.5/weather?lat=43.0147&lon=131.8642&appid= HTTP/1.1\r\n"
                        "Host: api.openweathermap.org\r\n"
                        "\r\n";

  my::http::Client cl;
  cl.connect({"api.openweathermap.org"});

  cl.send(my::http::Request::parse(request));

  auto answer = cl.receive();

  std::cout << std::string(answer.dump().first.get(), answer.dump().second);

  return 0;
}
