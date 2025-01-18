// #include "libmyhttp/myhttp.hpp"
// #include <nlohmann/json.hpp>
#include "libmyserial/myserial.hpp"
#include <chrono>
#include <iostream>
#include <thread>

int main(int argc, char **argv) {
  my::Serial port("/dev/pts/7");

  port.set_baudrate(my::Serial::BaudRate::BR_115200);
  port.set_parity(my::Serial::Parity::COM_PARITY_EVEN);
  port.set_bytesize(my::Serial::ByteSize::SIZE_8);
  port.set_stopbit(my::Serial::StopBits::STOPBIT_ONE);

  port.setup();

  port.flush();

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  std::string buf;
  while (true) {
    int res = port.read(buf);
    std::cout << "readed " << res << " size " << buf.size() << '\n';
    std::cout << buf;
    // for (int i = 0; i < buf.size(); ++i) {
    //   std::cout << i << ' ' << int(buf[i]) << '\n';
    // }
    std::cout << '\n';
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  return 0;
}
