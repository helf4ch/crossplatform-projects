#pragma once

#include <memory>
#include <netinet/in.h>
#include <string>
#include <vector>
#include <set>

namespace my::http {

class Adress {
public:

  Adress();
  
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

  Header(const std::string &key, const std::string &value);

  ~Header() = default;

  const std::string get_str() const;

  friend bool operator<(const Header &lhs, const Header &rhs);

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

  Request() = default;

  ~Request() = default;

  void set_adress(const Adress &addr);
  const Adress &get_adress() const;

  void set_type(const request_t type);
  const request_t get_type() const;

  void set_url(const std::string &url);
  const std::string &get_url() const;

  void add_header(const Header &header);
  const std::set<Header> &get_headers() const;

  void set_body(const char *body, const int body_lenght);
  const std::pair<const char*, int> get_body() const;

  const std::pair<std::unique_ptr<char[]>, int> dump();

  static const std::string get_type_string(request_t type);

  static Request parse(const std::string &request);

private:
  class RequestImpl;

  std::shared_ptr<RequestImpl> request;
};

class Response {
public:
  enum class response_t {
    UNKNOWN,
    INFO,
    SUCCESS,
    REDIRECTION,
    CLIENT_ERR,
    SERVER_ERR
  };

  Response(const int status, const std::string &phrase,
           std::vector<Header> headers, const char *body = nullptr,
           const int body_lenght = 0);

  ~Response() = default;

  void set_status(const int status, const std::string &phrase);

  const response_t get_type() const;

  const std::pair<int, const std::string&> get_status() const;

  const std::string &get_response() const;

  void add_header(const Header &header);

  const std::vector<Header> &get_headers() const;

  void set_body(const char *body, const int body_lenght);

  const std::pair<const char*, int> get_body() const;

  const std::pair<const char*, int> get_msg();

private:
  class ResponseImpl;

  std::shared_ptr<ResponseImpl> response;
};

class ResponseParser {
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
