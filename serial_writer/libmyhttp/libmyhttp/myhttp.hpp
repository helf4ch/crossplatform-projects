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

  Header(const std::string &key, const std::string &value);

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

  Request(const Adress &addr, const char *msg, const int size);

  ~Request() = default;

  const Adress &get_adress() const;

  const std::string &get_request() const;

  const std::vector<Header> &get_headers() const;

  const std::pair<const char*, int> get_body() const;

  const std::pair<const char*, int> get_msg() const;

  static const std::string get_type_string(request_t type);

  static Request gen(const request_t type, const Adress &addr,
                     const std::string &path, std::vector<Header> headers,
                     const std::vector<Param> &params, const char *body = NULL,
                     const int body_lenght = 0);

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

  const response_t get_type() const;

  const std::string &get_response() const;

  const std::vector<Header> &get_headers() const;

  const std::pair<const char*, int> get_body() const;

  const std::pair<const char*, int> get_msg() const;

private:
  class ResponseImpl;

  std::shared_ptr<ResponseImpl> response;
};

class ResponseParser : public Response {
public:
  enum class step_t {
    INFO,
    HEADERS,
    BODY
  }; 

  ResponseParser();

  ~ResponseParser() = default;

  bool parse_line(const std::string &line);

  const step_t get_cur_step() const;

  void set_status(const int status, const std::string &phrase);

  void add_header(const Header &header);

  void set_body(const char *body, const int body_lenght);

private:
  class ResponseParserImpl;

  std::shared_ptr<ResponseParserImpl> parser;
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
