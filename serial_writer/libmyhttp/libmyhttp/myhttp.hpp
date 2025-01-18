#pragma once

#include <memory>
#include <string>
#include <set>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <sys/socket.h>
#include <asm/termbits.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#endif

namespace my::http {
  
class Adress {
public:

  Adress();
  
  Adress(const std::string &name, int port = 80);

  ~Adress() = default;

  const struct addrinfo *get_addr() const;

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
  const std::pair<std::shared_ptr<char[]>, int> get_body() const;

  const std::pair<std::shared_ptr<char[]>, int> dump() const;

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
  const std::pair<std::shared_ptr<char[]>, int> get_body() const;

  const std::pair<std::shared_ptr<char[]>, int> dump() const;

  static Response parse(const std::string &request);

private:
  class ResponseImpl;

  std::shared_ptr<ResponseImpl> response;
};

class Connection {
public:
  Connection();

  ~Connection() = default;

  int get_socket();


private:
  class ConnectionImpl;

  std::shared_ptr<ConnectionImpl> conn;
};

class Client {
public:
  Client();

  ~Client() = default;

  void connect(const Adress &addr);

  void send(const Request &req);

  Response receive();

private:
  class ClientImpl;

  std::shared_ptr<ClientImpl> client;
};

class Server {};

} // namespace my::http
