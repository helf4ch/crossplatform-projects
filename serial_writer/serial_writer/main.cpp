#include "libmyhttp/myhttp.hpp"
#include "libmyserial/myserial.hpp"
#include <nlohmann/json.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <thread>

int main(int argc, char **argv) {
  if (argc < 5) {
    std::cout << "Usage: [port] [api_key] [lat] [lon]\n";
    return 0;
  }

  std::string com = argv[1];
  std::string appid = argv[2];
  std::string lat = argv[3];
  std::string lon = argv[4];

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

  nlohmann::json answer_js = nlohmann::json::parse(
      std::string(answer.get_body().first.get(), answer.get_body().second));

  std::cout << '\n' << std::endl;

  float temp = answer_js["main"]["temp"];
  temp -= 273;

  float pressure = answer_js["main"]["pressure"];
  float humidity = answer_js["main"]["humidity"];
  float wind_speed = answer_js["wind"]["speed"];

  float feels_like = answer_js["main"]["feels_like"];
  feels_like -= 273;

  nlohmann::json send_js;

  send_js["temp"] = temp;
  send_js["pressure"] = pressure;
  send_js["humidity"] = humidity;
  send_js["wind_speed"] = wind_speed;
  send_js["feels_like"] = feels_like;

  std::cout << send_js.dump();

  my::Serial port(com);

  port.set_baudrate(my::Serial::BaudRate::BR_115200);
  port.set_parity(my::Serial::Parity::COM_PARITY_EVEN);
  port.set_bytesize(my::Serial::ByteSize::SIZE_8);
  port.set_stopbit(my::Serial::StopBits::STOPBIT_ONE);

  port.setup();

  port << send_js.dump();

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  return 0;
}
