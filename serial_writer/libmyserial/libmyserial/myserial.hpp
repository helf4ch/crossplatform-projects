#pragma once

#include <memory>

#if defined(WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace my {

class Serial {
public:
  enum class BaudRate {
#ifdef _WIN32
    BR_4800 = CBR_4800,
    BR_9600 = CBR_9600,
    BR_19200 = CBR_19200,
    BR_38400 = CBR_38400,
    BR_57600 = CBR_57600,
    BR_115200 = CBR_115200,
#else
    BR_4800 = B4800,
    BR_9600 = B9600,
    BR_19200 = B19200,
    BR_38400 = B38400,
    BR_57600 = B57600,
    BR_115200 = B115200,
#endif
  };

  enum class Parity {
#ifdef _WIN32
    COM_PARITY_NONE = NOPARITY,
    COM_PARITY_ODD = ODDPARITY,
    COM_PARITY_EVEN = EVENPARITY,
#else
    COM_PARITY_NONE,
    COM_PARITY_ODD,
    COM_PARITY_EVEN,
#endif
  };

  enum class StopBits {
#ifdef _WIN32
    STOPBIT_ONE = ONESTOPBIT,
    STOPBIT_TWO = TWOSTOPBITS
#else
    STOPBIT_ONE,
    STOPBIT_TWO
#endif
  };

  enum class ByteSize {
#ifdef _WIN32
    SIZE_5 = 5,
    SIZE_6 = 6,
    SIZE_7 = 7,
    SIZE_8 = 8,
#else
    SIZE_5 = CS5,
    SIZE_6 = CS6,
    SIZE_7 = CS7,
    SIZE_8 = CS8,
#endif
  };

  Serial(const std::string &name);

  ~Serial() = default;

  void set_baudrate(BaudRate param);
  BaudRate get_baudrate() const;

  void set_parity(Parity param);
  Parity get_parity() const;

  void set_stopbit(StopBits param);
  StopBits get_stopbit() const;

  void set_bytesize(ByteSize param);
  ByteSize get_bytesize() const;

  void set_timeout(size_t millisec);
  size_t get_timeout() const;

  void setup();

  int read(char *buf, size_t count);
  int read(std::string &buf);

  int write(const char *buf, size_t count);
  int write(const std::string &buf);

  void flush();

  Serial &operator>>(std::string &data);
  Serial &operator<<(const std::string &data);
  
private:
  class SerialImpl;

  std::shared_ptr<SerialImpl> serial;
};

} // namespace my
