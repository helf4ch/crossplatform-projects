#pragma once

#include <netinet/in.h>
#include <string>

namespace my {

class Adress {
public: 
  Adress(const struct sockaddr_in &addr);

  ~Adress();

  static Adress get_by_name(const std::string &name, int port);

  static Adress get_by_ip(const std::string &ip, int port);

  const struct sockaddr_in &get_addr() const; 
 
private:
  class AdressImpl;

  AdressImpl *adress;
};

class Request {
public:
  enum class requets_t {
    GET,
    POST
  };
  
  Request();

  ~Request();

private:
  class RequestImpl;

  RequestImpl *request;
};

class Response {
public:
  enum class response_t {
    R_201,
    R_401
  };
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

class HTTPServer {
  
};

}
