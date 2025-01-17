// #include <cerrno>
// #include <sys/types.h>
// #include <unistd.h>
// #include <sys/stat.h>
// #include <fcntl.h>

#include "libmyhttp/myhttp.hpp"
#include "nlohmann/json_fwd.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

int main(int argc, char **argv) {
  // auto d = open("/dev/pts/4", O_WRONLY);

  // if (d == 0) {
  //   std::cout << "error " << errno;
  // }

  // char buf[] = "hello\n";
  // write(d, buf, 10);

  if (argc < 4) {
    std::cout << "Usage: [api_key] [lat] [lon]\n";
    return 0;
  }

  std::string appid = argv[1];
  std::string lat = argv[2];
  std::string lon = argv[3];

  std::stringstream ss;
  ss << "GET /data/2.5/weather?lat=" << lat << "&lon=" << lon
     << "&appid=" << appid << " HTTP/1.1\r\n"
     << "Host: api.openweathermap.org\r\n"
     << "\r\n";

  std::string request = ss.str();

  my::http::Client cl;
  cl.connect({"api.openweathermap.org"});

  cl.send(my::http::Request::parse(request));

  auto answer = cl.receive();

  std::cout << std::string(answer.dump().first.get(), answer.dump().second);

  nlohmann::json js = nlohmann::json::parse(
      std::string(answer.get_body().first.get(), answer.get_body().second));

  std::cout << '\n' << std::endl;

  float temp = js["main"]["temp"];
  temp -= 273;

  float feels_like = js["main"]["feels_like"];
  feels_like -= 273;

  float pressure = js["main"]["pressure"];
  float humidity = js["main"]["humidity"];

  std::cout << "temp: " << temp << '\n';
  std::cout << "feels_like: " << feels_like << '\n';
  std::cout << "pressure: " << pressure << '\n';
  std::cout << "humidity: " << humidity << '\n';

  return 0;
}
