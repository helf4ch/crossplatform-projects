#include "libmyhttp/myhttp.hpp"
#include <nlohmann/json.hpp>
#include "libmyserial/myserial.hpp"
#include <chrono>
#include <iostream>
#include <thread>
#include <sstream>

#define READ_TIME_MIN 1

std::string get_ctime_string() {
  std::stringstream buffer;
  std::time_t t = std::time(0);
  std::tm *now = std::localtime(&t);
  buffer << std::put_time(now, "%d.%m.%Y %H:%M:%S");
  return buffer.str();
}

int main(int argc, char **argv) {
  // if (argc < 2) {
  //   std::cout << "Usage: [port]\n";
  //   return 0;
  // }

  // std::string com = argv[1];

  // my::Serial port(com);

  // port.set_baudrate(my::Serial::BaudRate::BR_115200);
  // port.set_parity(my::Serial::Parity::COM_PARITY_EVEN);
  // port.set_bytesize(my::Serial::ByteSize::SIZE_8);
  // port.set_stopbit(my::Serial::StopBits::STOPBIT_ONE);
  // port.set_timeout(0);

  // port.setup();

  // while (true) {
  //   std::this_thread::sleep_for(std::chrono::seconds(READ_TIME_MIN * 60));

  //   std::cout << "===============\n";

  //   std::cout << "Wakeup at " << get_ctime_string() << "\n\n";

  //   std::string buf;
  //   int res = port.read(buf);
  //   if (res > 0) {
  //     if (buf[res - 1] != '\n') {
  //       std::cout << "read failed\n";
  //       port.flush();
  //     } else {
  //       std::cout << "Readed from port:\n";
  //       std::cout << buf;
  //     }
  //   } else {
  //     std::cout << "Got nothing\n";
  //   }

  //   std::cout << "===============\n\n";
  // }

  my::http::Server server({"127.0.0.1", 8080});  
  server.handle();

  return 0;
}
