#include "myserial.hpp"

#include "libmycommon/mycommon.hpp"
#include <fcntl.h>
#include <memory>
#include <termios.h>

class my::Serial::SerialImpl {
public:
  ~SerialImpl() {
    if (is_open) {
      close(port);
    }
  }

#ifdef _WIN32
  HANDLE port;
#else
  int port;
#endif

  bool is_open;

#ifdef _WIN32
  DCB settings;
#else
  struct termios settings;
#endif

  BaudRate rate;
  Parity parity;
  StopBits stopbit;
  ByteSize bytesize;
};

my::Serial::Serial(const std::string &name) {
  serial = std::make_shared<SerialImpl>();

#ifdef _WIN32

#else
  serial->port = ::open(name.c_str(), O_RDWR | O_NOCTTY);
  if (serial->port < 0) {
    throw my::common::Exception("Error in my::Serial::Serial.", errno);
  }

  serial->is_open = true;

  tcgetattr(serial->port, &serial->settings);
#endif
}

void my::Serial::set_baudrate(BaudRate param) {
  serial->rate = param;
#ifdef _WIN32

#else
  cfsetspeed(&serial->settings, static_cast<unsigned>(serial->rate));
#endif
}

my::Serial::BaudRate my::Serial::get_baudrate() const { return serial->rate; }

void my::Serial::set_parity(Parity param) {
  serial->parity = param;
#ifdef _WIN32

#else
  if (serial->parity == Parity::COM_PARITY_NONE) {
    serial->settings.c_cflag &= ~PARENB;
  } else {
    serial->settings.c_cflag |= PARENB;
  }
#endif
}

my::Serial::Parity my::Serial::get_parity() const { return serial->parity; }

void my::Serial::set_stopbit(StopBits param) {
  serial->stopbit = param;
#ifdef _WIN32

#else
  if (serial->stopbit == StopBits::STOPBIT_TWO) {
    serial->settings.c_cflag |= CSTOPB;
  } else {
    serial->settings.c_cflag &= ~CSTOPB;
  }
#endif
}

my::Serial::StopBits my::Serial::get_stopbit() const { return serial->stopbit; }

void my::Serial::set_bytesize(ByteSize param) {
  serial->bytesize = param;
#ifdef _WIN32

#else
  serial->settings.c_cflag &= ~CSIZE;
  serial->settings.c_cflag |= static_cast<unsigned>(serial->bytesize);
#endif
}

my::Serial::ByteSize my::Serial::get_bytesize() const {
  return serial->bytesize;
}

void my::Serial::setup() {
#ifdef _WIN32

#else
  serial->settings.c_cc[VMIN] = 1;
  serial->settings.c_cc[VTIME] = 10;
  serial->settings.c_cflag &= ~CRTSCTS;
  serial->settings.c_cflag |= CLOCAL | CREAD;
  serial->settings.c_iflag &= ~(IXON | IXOFF | IXANY);
  serial->settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  serial->settings.c_oflag &= ~OPOST;

  int res = tcsetattr(serial->port, TCSANOW, &serial->settings);
  if (res) {
    throw my::common::Exception("Error in my::Serial::setup.", errno);
  }
#endif
}

int my::Serial::read(char *buf, size_t count) {
#ifdef _WIN32

#else
  int res = ::read(serial->port, buf, count);
  if (res < 0) {
    throw my::common::Exception("Error in my::Serial::read.", errno);
  }

  return res;
#endif
}

int my::Serial::read(std::string &buf) {
  buf.reserve(1024);
  int res = read(buf.data(), buf.capacity());
  buf.resize(res);
  return res;
}

int my::Serial::write(const char *buf, size_t count) {
#ifdef _WIN32

#else
  int writed = 0;
  do {
    int bytes = ::write(serial->port, buf, count);

    if (bytes < 0) {
      throw my::common::Exception("Error in my::Serial::write.", errno);
    }

    writed += bytes;
  } while (writed != count);

  return writed;
#endif
}

int my::Serial::write(const std::string &buf) {
  int res = write(const_cast<char *>(buf.c_str()), buf.size());
  return res;
}

void my::Serial::flush() {
#ifdef _WIN32

#else
  int res = tcflush(serial->port, TCIOFLUSH);
  if (res) {
    throw my::common::Exception("Error in my::Serial::flush.", errno);
  }
#endif
}

my::Serial &my::Serial::operator>>(std::string &data) {
  read(data);
  return *this;
}

my::Serial &my::Serial::operator<<(const std::string &data) {
  write(data);
  return *this;
}
