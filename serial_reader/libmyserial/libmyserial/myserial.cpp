#include "myserial.hpp"

#include "libmycommon/mycommon.hpp"
#include <memory>

class my::Serial::SerialImpl {
public:
  ~SerialImpl() {
    if (is_open) {
#ifdef _WIN32
      CloseHandle(port);
#else
      tcflush(port, TCIOFLUSH);
      close(port);
#endif
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
  size_t timeout = 0;
};

my::Serial::Serial(const std::string &name) {
  serial = std::make_shared<SerialImpl>();

#ifdef _WIN32
  std::string system_name = "\\\\.\\" + name;

  serial->port = CreateFileA(system_name.c_str(), GENERIC_READ | GENERIC_WRITE,
                             0, NULL, OPEN_EXISTING, 0, NULL);
  if (serial->port == INVALID_HANDLE_VALUE) {
    throw my::common::Exception("Error in my::Serial::Serial.", GetLastError());
  }

  serial->settings.DCBlength = sizeof(DCB);
  GetCommState(serial->port, &serial->settings);
#else
  serial->port = ::open(name.c_str(), O_RDWR | O_NOCTTY);
  if (serial->port < 0) {
    throw my::common::Exception("Error in my::Serial::Serial.", errno);
  }

  tcgetattr(serial->port, &serial->settings);
#endif
  serial->is_open = true;
}

void my::Serial::set_baudrate(BaudRate param) {
  serial->rate = param;
#ifdef _WIN32
  serial->settings.BaudRate = static_cast<BYTE>(serial->rate);
#else
  cfsetspeed(&serial->settings, static_cast<unsigned>(serial->rate));
#endif
}

my::Serial::BaudRate my::Serial::get_baudrate() const { return serial->rate; }

void my::Serial::set_parity(Parity param) {
  serial->parity = param;
#ifdef _WIN32
  serial->settings.Parity = static_cast<BYTE>(serial->parity);
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
  serial->settings.StopBits = static_cast<BYTE>(serial->stopbit);
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
  serial->settings.ByteSize = static_cast<BYTE>(serial->bytesize);
#else
  serial->settings.c_cflag &= ~CSIZE;
  serial->settings.c_cflag |= static_cast<unsigned>(serial->bytesize);
#endif
}

my::Serial::ByteSize my::Serial::get_bytesize() const {
  return serial->bytesize;
}

void my::Serial::set_timeout(size_t millisec) {
  serial->timeout = millisec;
#ifdef _WIN32
  COMMTIMEOUTS tmts = {0};

  GetCommTimeouts(serial->port, &tmts);

  if (serial->timeout > 0) {
    tmts.ReadIntervalTimeout = 0;
  } else {
    tmts.ReadIntervalTimeout = MAXDWORD;
  }

  tmts.ReadTotalTimeoutConstant = serial->timeout;
  tmts.ReadTotalTimeoutMultiplier = 0;
  tmts.WriteTotalTimeoutConstant = serial->timeout;
  tmts.WriteTotalTimeoutMultiplier = 0;

  SetCommTimeouts(serial->port, &tmts);
#else
  serial->settings.c_cc[VTIME] = serial->timeout / 100;
#endif
}

size_t my::Serial::get_timeout() const { return serial->timeout; }

void my::Serial::setup() {
#ifdef _WIN32
  serial->settings.fBinary = TRUE;
  serial->settings.fAbortOnError = FALSE;
  serial->settings.fOutxCtsFlow = FALSE;
  serial->settings.fRtsControl = RTS_CONTROL_DISABLE;
  serial->settings.fOutxDsrFlow = FALSE;
  serial->settings.fDsrSensitivity = FALSE;
  serial->settings.fDtrControl = DTR_CONTROL_DISABLE;
  serial->settings.fInX = FALSE;
  serial->settings.fOutX = FALSE;
  serial->settings.fErrorChar = FALSE;
  serial->settings.fNull = FALSE;

  BOOL status = SetCommState(serial->port, &serial->settings);

  if (status == FALSE) {
    throw my::common::Exception("Error in my::Serial::setup.", GetLastError());
  }

#else
  serial->settings.c_cc[VMIN] = 0;
  serial->settings.c_cflag &= ~CRTSCTS;
  serial->settings.c_cflag |= CLOCAL | CREAD;
  serial->settings.c_iflag &= ~(IXON | IXOFF | IXANY);
  serial->settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  serial->settings.c_oflag &= ~OPOST;

  int res = tcsetattr(serial->port, TCSANOW, &serial->settings);
  if (res) {
    throw my::common::Exception("Error in my::Serial::setup.", errno);
  }

  flush();
#endif
}

int my::Serial::read(char *buf, size_t count) {
  int readed = 0;
  do {
#ifdef _WIN32
    DWORD bytes = 0;
    if (!::ReadFile(serial->port, buf + readed, count - readed, &bytes, NULL)) {
      throw my::common::Exception("Error in my::Serial::read.", GetLastError());
    }

#else
    int bytes = ::read(serial->port, buf + readed, count - readed);

    if (bytes < 0) {
      throw my::common::Exception("Error in my::Serial::read.", errno);
    }

#endif
    if (bytes == 0) {
      break;
    }

    readed += bytes;
  } while (readed < count);

  return readed;
}

int my::Serial::read(std::string &buf) {
  buf.resize(512);
  int res = read(buf.data(), buf.capacity());
  buf.resize(res);
  return res;
}

int my::Serial::write(const char *buf, size_t count) {
  int writed = 0;
  do {
#ifdef _WIN32
    DWORD bytes = 0;
    if (!::WriteFile(serial->port, buf + writed, count - writed, &bytes,
                     NULL)) {
      throw my::common::Exception("Error in my::Serial::read.", GetLastError());
    }

#else
    int bytes = ::write(serial->port, buf + writed, count - writed);

    if (bytes < 0) {
      throw my::common::Exception("Error in my::Serial::write.", errno);
    }

#endif
    if (bytes == 0) {
      break;
    }

    writed += bytes;
  } while (writed < count);

  return writed;
}

int my::Serial::write(const std::string &buf) {
  int res = write(buf.c_str(), buf.size());
  return res;
}

void my::Serial::flush() {
#ifdef _WIN32
  if (!PurgeComm(serial->port, PURGE_TXCLEAR | PURGE_RXCLEAR)) {
    throw my::common::Exception("Error in my::Serial::flush.", GetLastError());
  }
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
