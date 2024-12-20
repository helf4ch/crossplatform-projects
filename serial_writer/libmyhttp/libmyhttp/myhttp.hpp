#pragma once

#include <fstream>
#include <netinet/in.h>
#include <string>

namespace my {

class Adress {
public:
  enum class adress_t { HOSTNAME, IP };

  Adress(const adress_t type, const struct sockaddr_in &addr);

  ~Adress();

  const struct sockaddr_in &get_addr() const;

  const adress_t get_type() const;

  static Adress get_by_name(const std::string &name, int port = 80);

  static Adress get_by_ip(const std::string &ip, int port = 80);

private:
  class AdressImpl;

  AdressImpl *adress;
};

class Header {
public:
  enum class header_t {
    UNKNOWN,
    Host,
    Date,
    Content_type,
    Content_lenght,
    Connection
  };

  Header(const header_t type, const std::string &value,
         const std::string &key = "");

  ~Header();

  const std::string get_str() const;

  friend std::ofstream &operator<<(std::ofstream &out, const Header &obj);

  static const std::string get_key_string(header_t type,
                                          const std::string &fail_str = "");

  static const header_t
  get_type_name(const std::string &key,
                const header_t fail_type = header_t::UNKNOWN);

private:
  class HeaderImpl;

  HeaderImpl *header;
};

class Request {
public:
  enum class request_t { GET, POST };

  Request(const std::string &request);

  ~Request();

  const std::string &get_request() const;

  static Request format(const request_t type);

private:
  class RequestImpl;

  RequestImpl *request;
};

class Response {
public:
  enum class response_t { R_201, R_401 };
};

class Socket {
public:
  Socket();

  ~Socket();

private:
  class SocketImpl;

  SocketImpl *socket;
};

class HTTPClient {
public:
  HTTPClient();

  ~HTTPClient();

private:
  class HTTPClientImpl;

  HTTPClient *client;
};

class HTTPServer {};

} // namespace my
