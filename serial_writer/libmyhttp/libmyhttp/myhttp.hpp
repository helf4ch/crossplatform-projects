#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
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

  void set_method(const std::string &method);
  const std::string get_method() const;

  void set_url(const std::string &url);
  const std::string &get_url() const;

  void add_param(const Param &param);
  Param &get_param(const std::string &key) const;
  const std::set<Param> &get_params() const;

  void set_http_ver(const std::string &ver);
  const std::string &get_http_ver() const;

  void add_header(const Header &header);
  Header &get_header(const std::string key) const;
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
  Header &get_header(const std::string key) const;
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
#ifdef _WIN32
  typedef SOCKET socket_t;
#else
  typedef int socket_t;
#endif

  Connection();

  Connection(const socket_t socket);

  ~Connection() = default;

  socket_t get_socket() const;

private:
  class ConnectionImpl;

  std::shared_ptr<ConnectionImpl> conn;
};

class Http {
public:
  Http(const Connection &conn);

  Http(const Adress &addr);

  ~Http() = default;

  void send(const Request &req) const;

  Response receive() const;

private:
  class HttpImpl;

  std::shared_ptr<HttpImpl> http;
};

class Client {
public:
  Client(const Connection &conn);

  ~Client() = default;

  void send(const Response &res) const;

  Request receive() const;

private:
  class ClientImpl;

  std::shared_ptr<ClientImpl> client;
};

class AHandler {
public:
  virtual void operator()(const Client &client, const Request &request) = 0;
};

struct Configuration {
  std::string url;
  std::map<std::string, std::shared_ptr<AHandler>> method_to_handler;

  friend bool operator<(const Configuration &lhs, const Configuration &rhs) {
    return lhs.url < rhs.url;
  }
};

class Server {
public:
  Server(const Adress &addr);

  ~Server() = default;

  void add_configuration(const Configuration &config);

  void handle() const;

private:
  class ServerImpl;

  std::shared_ptr<ServerImpl> server;
};

} // namespace my::http
