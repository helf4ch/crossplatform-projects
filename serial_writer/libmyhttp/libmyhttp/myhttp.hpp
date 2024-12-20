#pragma once

#include <memory>
#include <netinet/in.h>
#include <string>
#include <vector>

namespace my::http {

class Adress {
public:
  Adress(const std::string &name, int port = 80);

  ~Adress() = default;

  const struct sockaddr_in &get_addr() const;

  const int get_port() const;

  const std::string &get_hostname() const;

private:
  class AdressImpl;

  std::shared_ptr<AdressImpl> adress;
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

  ~Header() = default;

  const std::string get_str() const;

  friend std::ostream &operator<<(std::ostream &out, const Header &obj);

  static const std::string get_type_string(header_t type,
                                           const std::string &fail_str = "");

  static const header_t
  get_type_type(const std::string &name,
                const header_t fail_type = header_t::UNKNOWN);

private:
  class HeaderImpl;

  std::shared_ptr<HeaderImpl> header;
};

class Param {
public:
  Param(const std::string &key, const std::string &value);

  ~Param() = default;

  const std::string get_str() const;

  friend std::ostream &operator<<(std::ostream &out, const Param &obj);

private:
  class ParamImpl;

  std::shared_ptr<ParamImpl> param;
};

class Request {
public:
  enum class request_t { GET, POST };

  Request(const Adress &addr, const std::string &request_str);

  ~Request() = default;

  const Adress &get_adress() const;

  const std::string get_str() const;

  static const std::string get_type_string(request_t type);

  static Request gen(const request_t type, const Adress &addr,
                     const std::string &path, std::vector<Header> headers,
                     const std::vector<Param> &params);

private:
  class RequestImpl;

  std::shared_ptr<RequestImpl> request;
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

class Client {
public:
  Client();

  ~Client();

private:
  class ClientImpl;

  ClientImpl *client;
};

class Server {};

} // namespace my::http
