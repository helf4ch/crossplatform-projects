// #include "libmyhttp/myhttp.hpp"
// #include <nlohmann/json.hpp>
#include "libmyserial/myserial.hpp"
#include <chrono>
#include <iostream>
#include <thread>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "Usage: [port]\n";
    return 0;
  }

  std::string com = argv[1];

  my::Serial port(com);

  port.set_baudrate(my::Serial::BaudRate::BR_115200);
  port.set_parity(my::Serial::Parity::COM_PARITY_EVEN);
  port.set_bytesize(my::Serial::ByteSize::SIZE_8);
  port.set_stopbit(my::Serial::StopBits::STOPBIT_ONE);

  port.setup();

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  std::string buf;
  while (true) {
    int res = port.read(buf);
    std::cout << "readed " << res << " got " << buf.size() << " cap " << buf.capacity() << '\n';
    std::cout << buf;
    std::cout << '\n';
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  return 0;
}
