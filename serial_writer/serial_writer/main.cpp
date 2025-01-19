#include "libmyhttp/myhttp.hpp"
#include "libmyserial/myserial.hpp"
#include <nlohmann/json.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#define WRITE_TIME_MIN 20

std::string get_ctime_string() {
  std::stringstream buffer;
  std::time_t t = std::time(0);
  std::tm *now = std::localtime(&t);
  buffer << std::put_time(now, "%d.%m.%Y %H:%M:%S");
  return buffer.str();
}

std::string get_data(const std::string &appid, const std::string &lat,
                     const std::string &lon) {
  std::stringstream ss;
  ss << "GET /data/2.5/weather?lat=" << lat << "&lon=" << lon
     << "&appid=" << appid << " HTTP/1.1\r\n"
     << "Host: api.openweathermap.org\r\n"
     << "\r\n";

  std::string request = ss.str();

  my::http::Http http({"api.openweathermap.org"});

  http.send(my::http::Request::parse(request));

  auto answer = http.receive();

  std::cout << "Request result:\n";
  std::cout << std::string(answer.dump().first.get(), answer.dump().second);

  nlohmann::json answer_js = nlohmann::json::parse(
      std::string(answer.get_body().first.get(), answer.get_body().second));

  std::cout << '\n' << std::endl;

  float temp = answer_js["main"]["temp"];
  temp -= 273;

  int pressure = answer_js["main"]["pressure"];
  int humidity = answer_js["main"]["humidity"];

  float wind_speed = answer_js["wind"]["speed"];

  float feels_like = answer_js["main"]["feels_like"];
  feels_like -= 273;

  nlohmann::json send_js;

  send_js["temperature"] = temp;
  send_js["pressure"] = pressure;
  send_js["humidity"] = humidity;
  send_js["wind_speed"] = wind_speed;
  send_js["feels_like"] = feels_like;

  return send_js.dump();
}

int main(int argc, char **argv) {
  if (argc < 5) {
    std::cout << "Usage: [port] [api_key] [lat] [lon]\n";
    return 0;
  }

  std::string com = argv[1];
  std::string appid = argv[2];
  std::string lat = argv[3];
  std::string lon = argv[4];

  my::Serial port(com);

  port.set_baudrate(my::Serial::BaudRate::BR_115200);
  port.set_parity(my::Serial::Parity::COM_PARITY_EVEN);
  port.set_bytesize(my::Serial::ByteSize::SIZE_8);
  port.set_stopbit(my::Serial::StopBits::STOPBIT_ONE);
  port.set_timeout(0);

  port.setup();

  while (true) {
    std::cout << "===============\n";

    std::cout << "Wakeup at " << get_ctime_string() << "\n\n";

    auto data = get_data(appid, lat, lon);

    std::cout << "Writing to port:\n";
    std::cout << data << '\n';

    port << data << "\n";

    std::cout << "===============\n\n";

    std::this_thread::sleep_for(std::chrono::seconds(WRITE_TIME_MIN * 60));
  }

  return 0;
}
