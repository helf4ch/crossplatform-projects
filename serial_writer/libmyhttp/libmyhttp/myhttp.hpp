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
  Header(const std::string &key, const std::string &value);

  ~Header() = default;

  const std::string &get_key() const;

  void set_value(const std::string &value);
  const std::string &get_value() const;

  const std::string get_str() const;

  friend bool operator<(const Header &lhs, const Header &rhs);

  friend std::ostream &operator<<(std::ostream &out, const Header &obj);

private:
  class HeaderImpl;

  std::shared_ptr<HeaderImpl> header;
};

class Param {
public:
  Param(const std::string &key, const std::string &value);

  ~Param() = default;

  const std::string &get_key() const;

  void set_value(const std::string &value);
  const std::string &get_value() const;

  const std::string get_str() const;

  friend bool operator<(const Param &lhs, const Param &rhs);

  friend std::ostream &operator<<(std::ostream &out, const Param &obj);

private:
  class ParamImpl;

  std::shared_ptr<ParamImpl> param;
};

class Request {
public:
  Request();

  ~Request() = default;

  void set_adress(const Adress &addr);
  const Adress &get_adress() const;

  void set_type(const std::string & type);
  const std::string get_type() const;

  void set_url(const std::string &url);
  const std::string &get_url() const;

  void add_param(const Param &param);
  Param &get_param(const std::string &key);
  const std::set<Param> &get_params() const;

  void set_http_ver(const std::string &ver);
  const std::string &get_http_ver() const;

  void add_header(const Header &header);
  Header &get_header(const std::string key);
  const std::set<Header> &get_headers() const;

  void set_body(const char *body, const int body_lenght);
  const std::pair<const char*, int> get_body() const;

  const std::pair<std::unique_ptr<char[]>, int> dump();

  static Request parse(const std::string &request);

private:
  class RequestImpl;

  std::shared_ptr<RequestImpl> request;
};

class Response {
public:
  Response();

  ~Response() = default;

  void set_adress(const Adress &addr);
  const Adress &get_adress() const;

  void set_http_ver(const std::string &ver);
  const std::string &get_http_ver() const;

  void set_code(const int code);
  const int get_code() const;

  void set_text(const std::string &text);
  const std::string &get_text() const;

  void add_header(const Header &header);
  Header &get_header(const std::string key);
  const std::set<Header> &get_headers() const;

  void set_body(const char *body, const int body_lenght);
  const std::pair<const char*, int> get_body() const;

  const std::pair<std::unique_ptr<char[]>, int> dump();

  static Response parse(const std::string &request);

private:
  class ResponseImpl;

  std::shared_ptr<ResponseImpl> response;
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
