#include "myhttp.hpp"

#include "libmycommon/mycommon.hpp"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

// my::http::Adress
class my::http::Adress::AdressImpl {
public:
  ~AdressImpl() { freeaddrinfo(addr); }

  int port;
  std::string hostname;
  struct addrinfo *addr;
};

my::http::Adress::Adress() { adress = std::make_shared<AdressImpl>(); }

my::http::Adress::Adress(const std::string &name, int port) {
  adress = std::make_shared<AdressImpl>();

  adress->port = port;
  adress->hostname = name;

  int status;
  struct addrinfo hints;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  status = getaddrinfo(name.c_str(), std::to_string(port).c_str(), &hints,
                       &adress->addr);

  if (status) {
    throw my::common::Exception("Error in my::http::Adress.", h_errno);
  }
}

const struct addrinfo *my::http::Adress::get_addr() const {
  return adress->addr;
}

const int my::http::Adress::get_port() const { return adress->port; }

const std::string &my::http::Adress::get_hostname() const {
  return adress->hostname;
}

// my::http::Header
class my::http::Header::HeaderImpl {
public:
  std::string key;
  std::string value;
};

my::http::Header::Header(const std::string &key, const std::string &value) {
  header = std::make_shared<HeaderImpl>();
  header->key = key;
  header->value = value;
}

const std::string &my::http::Header::get_key() const { return header->key; }

void my::http::Header::set_value(const std::string &value) {
  header->value = value;
}

const std::string &my::http::Header::get_value() const { return header->value; }

const std::string my::http::Header::get_str() const {
  std::stringstream ss;
  ss << header->key << ": " << header->value;
  return ss.str();
}

bool my::http::operator<(const Header &lhs, const Header &rhs) {
  return lhs.header->key < rhs.header->key;
}

std::ostream &my::http::operator<<(std::ostream &out,
                                   const my::http::Header &obj) {
  out << obj.get_str();
  return out;
}

// my::http::Param
class my::http::Param::ParamImpl {
public:
  std::string key;
  std::string value;
};

my::http::Param::Param(const std::string &key, const std::string &value) {
  param = std::make_shared<ParamImpl>();
  param->key = key;
  param->value = value;
}

const std::string &my::http::Param::get_key() const { return param->key; }

void my::http::Param::set_value(const std::string &value) {
  param->value = value;
}

const std::string &my::http::Param::get_value() const { return param->value; }

const std::string my::http::Param::get_str() const {
  std::stringstream ss;
  ss << param->key << '=' << param->value;
  return ss.str();
}

bool my::http::operator<(const Param &lhs, const Param &rhs) {
  return lhs.get_key() < rhs.get_key();
}

std::ostream &my::http::operator<<(std::ostream &out,
                                   const my::http::Param &obj) {
  out << obj.get_str();
  return out;
}

// my::http::Request
class my::http::Request::RequestImpl {
public:
  std::unique_ptr<Adress> addr;

  std::string method;
  std::string url;
  std::string http_ver = "HTTP/1.1";
  std::set<Param> params;
  std::set<Header> headers;
  std::shared_ptr<char[]> body;
  int body_size = 0;
};

my::http::Request::Request() { request = std::make_shared<RequestImpl>(); }

void my::http::Request::set_adress(const my::http::Adress &addr) {
  *request->addr = addr;
}

const my::http::Adress &my::http::Request::get_adress() const {
  return *request->addr;
}

void my::http::Request::set_method(const std::string &method) {
  request->method = method;
}

const std::string my::http::Request::get_method() const {
  return request->method;
}

void my::http::Request::set_url(const std::string &url) { request->url = url; }

void my::http::Request::add_param(const Param &param) {
  request->params.insert(param);
}

my::http::Param &my::http::Request::get_param(const std::string &key) const {
  auto it = std::find_if(request->params.begin(), request->params.end(),
                         [key](auto &it) { return it.get_key() == key; });

  if (it == request->params.end()) {
    throw std::out_of_range("No such param.");
  }

  return const_cast<Param &>(*it);
}

const std::set<my::http::Param> &my::http::Request::get_params() const {
  return request->params;
}

const std::string &my::http::Request::get_url() const { return request->url; }

void my::http::Request::set_http_ver(const std::string &ver) {
  request->http_ver = ver;
}

const std::string &my::http::Request::get_http_ver() const {
  return request->http_ver;
}

my::http::Header &my::http::Request::get_header(const std::string key) const {
  auto it = std::find_if(request->headers.begin(), request->headers.end(),
                         [key](auto &it) { return it.get_key() == key; });

  if (it == request->headers.end()) {
    throw std::out_of_range("No such param.");
  }

  return const_cast<Header &>(*it);
}

void my::http::Request::add_header(const Header &header) {
  request->headers.insert(header);
}

const std::set<my::http::Header> &my::http::Request::get_headers() const {
  return request->headers;
}

void my::http::Request::set_body(const char *body, const int body_lenght) {
  request->body = std::shared_ptr<char[]>(new char[body_lenght]);
  memcpy(request->body.get(), body, body_lenght);
  request->body_size = body_lenght;
}

const std::pair<std::shared_ptr<char[]>, int>
my::http::Request::get_body() const {
  return {request->body, request->body_size};
}

const std::pair<std::shared_ptr<char[]>, int> my::http::Request::dump() const {
  std::stringstream ss;

  ss << request->method << ' ' << request->url << '?';

  for (auto &it : request->params) {
    ss << it << '&';
  }

  ss << ' ' << request->http_ver << "\r\n";

  for (auto &it : request->headers) {
    ss << it << "\r\n";
  }

  ss << "\r\n";

  std::string msg = ss.str();

  size_t size = msg.size() + request->body_size;
  std::shared_ptr<char[]> ptr(new char[size]);

  memcpy(ptr.get(), msg.c_str(), msg.size());
  memcpy(ptr.get() + msg.size(), request->body.get(), request->body_size);

  return {ptr, size};
}

std::vector<std::string> split(std::string s, std::string delimiter) {
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  std::string token;
  std::vector<std::string> res;

  while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back(token);
  }

  res.push_back(s.substr(pos_start));
  return res;
}

my::http::Request my::http::Request::parse(const std::string &request) {
  auto lines = split(request, "\r\n");

  Request req;

  std::stringstream ss(lines[0]);
  std::string buf;

  ss >> buf;
  req.set_method(buf);

  ss >> buf;
  auto url_split = split(buf, "?");
  req.set_url(url_split[0]);

  auto param_pos = buf.find("?");
  req.set_url(buf.substr(0, param_pos));

  if (param_pos != std::string::npos) {
    auto param_split = split(buf.substr(param_pos + 1), "&");

    for (int i = 0; i < param_split.size(); ++i) {
      auto pos = param_split[i].find("=");
      std::string key = param_split[i].substr(0, pos);
      std::string value = param_split[i].substr(pos + 1, param_split[i].size());
      req.add_param({key, value});
    }
  }

  ss >> buf;
  req.set_http_ver(buf);

  for (int i = 1; i < lines.size() - 2; ++i) {
    auto pos = lines[i].find(":");
    std::string key = lines[i].substr(0, pos);
    std::string value = lines[i].substr(pos + 2);
    req.add_header({key, value});
  }

  return req;
}

class my::http::Response::ResponseImpl {
public:
  std::unique_ptr<Adress> addr;

  std::string http_ver = "HTTP/1.1";
  int code;
  std::string text;
  std::set<Header> headers;
  std::shared_ptr<char[]> body;
  int body_size = 0;
};

my::http::Response::Response() { response = std::make_shared<ResponseImpl>(); }

void my::http::Response::set_adress(const my::http::Adress &addr) {
  *response->addr = addr;
}

const my::http::Adress &my::http::Response::get_adress() const {
  return *response->addr;
}

void my::http::Response::set_http_ver(const std::string &ver) {
  response->http_ver = ver;
}

const std::string &my::http::Response::get_http_ver() const {
  return response->http_ver;
}

void my::http::Response::set_code(const int code) { response->code = code; }

const int my::http::Response::get_code() const { return response->code; }

void my::http::Response::set_text(const std::string &text) {
  response->text = text;
}

const std::string &my::http::Response::get_text() const {
  return response->text;
}

void my::http::Response::add_header(const Header &header) {
  response->headers.insert(header);
}

my::http::Header &my::http::Response::get_header(const std::string key) const {
  auto it = std::find_if(response->headers.begin(), response->headers.end(),
                         [key](auto &it) { return it.get_key() == key; });

  if (it == response->headers.end()) {
    throw std::out_of_range("No such param.");
  }

  return const_cast<Header &>(*it);
}

const std::set<my::http::Header> &my::http::Response::get_headers() const {
  return response->headers;
}

void my::http::Response::set_body(const char *body, const int body_lenght) {
  response->body = std::shared_ptr<char[]>(new char[body_lenght]);
  memcpy(response->body.get(), body, body_lenght);
  response->body_size = body_lenght;
}

const std::pair<std::shared_ptr<char[]>, int>
my::http::Response::get_body() const {
  return {response->body, response->body_size};
}

const std::pair<std::shared_ptr<char[]>, int> my::http::Response::dump() const {
  std::stringstream ss;

  ss << response->http_ver << ' ' << response->code << ' ' << response->text
     << "\r\n";

  for (auto &it : response->headers) {
    ss << it << "\r\n";
  }

  ss << "\r\n";

  std::string msg = ss.str();

  size_t size = msg.size() + response->body_size;
  std::shared_ptr<char[]> ptr(new char[size]);

  memcpy(ptr.get(), msg.c_str(), msg.size());
  memcpy(ptr.get() + msg.size(), response->body.get(), response->body_size);

  return {ptr, size};
}

my::http::Response my::http::Response::parse(const std::string &request) {
  auto lines = split(request, "\r\n");

  Response res;

  std::stringstream ss(lines[0]);
  std::string buf;

  ss >> buf;
  res.set_http_ver(buf);

  ss >> buf;
  res.set_code(std::stoi(buf));

  std::getline(ss, buf);
  res.set_text(buf);

  for (int i = 1; i < lines.size() - 2; ++i) {
    auto pos = lines[i].find(":");
    std::string key = lines[i].substr(0, pos);
    std::string value = lines[i].substr(pos + 2);
    res.add_header({key, value});
  }

  return res;
}

#ifdef _WIN32
class _WSA_CLASS {
public:
  _WSA_CLASS() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
  }

  ~_WSA_CLASS() { WSACleanup(); }
};

std::unique_ptr<_WSA_CLASS> _wsa = std::make_unique<_WSA_CLASS>();
#endif

// Connection
class my::http::Connection::ConnectionImpl {
public:
  ~ConnectionImpl() { close(socket); }

  socket_t socket;
};

my::http::Connection::Connection() {
  conn = std::make_shared<ConnectionImpl>();
  conn->socket = ::socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
  if (conn->socket == INVALID_SOCKET) {
    throw my::common::Exception("Error in my::http::Connection::Connection.",
                                WSAGetLastError());
  }
#else
  if (conn->socket == -1) {
    throw my::common::Exception("Error in my::http::Connection::Connection.",
                                errno);
  }
#endif
}

my::http::Connection::Connection(const socket_t socket) {
  conn = std::make_shared<ConnectionImpl>();
  conn->socket = socket;
}

my::http::Connection::socket_t my::http::Connection::get_socket() const {
  return conn->socket;
}

// Http
class my::http::Http::HttpImpl {
public:
  Connection conn;
};

my::http::Http::Http(const Connection &conn) {
  http = std::make_shared<HttpImpl>();
  http->conn = conn;
}

my::http::Http::Http(const Adress &addr) {
  http = std::make_shared<HttpImpl>();

  int res = ::connect(http->conn.get_socket(), addr.get_addr()->ai_addr,
                      addr.get_addr()->ai_addrlen);
#ifdef _WIN32
  if (res == SOCKET_ERROR) {
    throw my::common::Exception("Error in my::http::Http::Http.",
                                WSAGetLastError());
  }
#else
  if (res) {
    throw my::common::Exception("Error in my::http::Http::Http.", errno);
  }
#endif
}

void my::http::Http::send(const Request &req) const {
  auto dump = req.dump();
  auto msg = dump.first;
  auto size = dump.second;

  int sent = 0;
  do {
    int bytes =
        ::send(http->conn.get_socket(), msg.get() + sent, size - sent, 0);

#ifdef _WIN32
    if (bytes == SOCKET_ERROR) {
      throw my::common::Exception("Error in my::http::Http::send.",
                                  WSAGetLastError());
    }
#else
    if (bytes < 0) {
      throw my::common::Exception("Error in my::http::Http::send.", errno);
    }
#endif

    if (bytes == 0) {
      break;
    }

    sent += bytes;
  } while (sent < size);
}

my::http::Response my::http::Http::receive() const {
  int bufsize = 1024;
  char *buf = new char[bufsize]();
  int received = 0;
  std::string end_of_read = "\r\n\r\n";
  int num_read = 1;
  while (true) {
    int bytes = ::recv(http->conn.get_socket(), buf + received, num_read, 0);

#ifdef _WIN32
    if (bytes == SOCKET_ERROR) {
      throw my::common::Exception("Error in my::http::Http::receive.",
                                  WSAGetLastError());
    }
#else
    if (bytes < 0) {
      throw my::common::Exception("Error in my::http::Http::receive.", errno);
    }
#endif

    received += num_read;

    if (bytes == 0) {
      break;
    }

    char *end_pos = strstr(buf, end_of_read.c_str());
    if (end_pos != NULL) {
      break;
    }

    if (received == bufsize) {
      int new_bufsize = bufsize * 2;
      char *new_buf = new char[new_bufsize]();
      strcpy(new_buf, buf);
      delete[] buf;
      bufsize = new_bufsize;
      buf = new_buf;
    }
  }

  auto res = Response::parse(buf);

  delete[] buf;

#ifdef _WIN32
  u_long len = 0;
  ioctlsocket(http->conn.get_socket(), FIONREAD, &len);
#else
  int len = 0;
  ioctl(http->conn.get_socket(), FIONREAD, &len);
#endif

  if (len == 0) {
    return res;
  }

  bufsize = 1024;
  buf = new char[bufsize]();
  received = 0;
  while (true) {
    int bytes =
        ::recv(http->conn.get_socket(), buf + received, len - received, 0);

#ifdef _WIN32
    if (bytes == SOCKET_ERROR) {
      throw my::common::Exception("Error in my::http::Http::receive.",
                                  WSAGetLastError());
    }
#else
    if (bytes < 0) {
      throw my::common::Exception("Error in my::http::Http::receive.", errno);
    }
#endif

    received += bytes;

    if (bytes == 0 || received == len) {
      break;
    }

    if (received == bufsize) {
      int new_bufsize = bufsize * 2;
      char *new_buf = new char[new_bufsize]();
      strcpy(new_buf, buf);
      delete[] buf;
      bufsize = new_bufsize;
      buf = new_buf;
    }
  }

  res.set_body(buf, received + 1);

  return res;
}

// Client
class my::http::Client::ClientImpl {
public:
  Connection conn;
};

my::http::Client::Client(const Connection &conn) {
  client = std::make_shared<ClientImpl>();
  client->conn = conn;
}

void my::http::Client::send(const Response &res) const {
  auto dump = res.dump();
  auto msg = dump.first;
  auto size = dump.second;

  int sent = 0;
  do {
    int bytes =
        ::send(client->conn.get_socket(), msg.get() + sent, size - sent, 0);

#ifdef _WIN32
    if (bytes == SOCKET_ERROR) {
      throw my::common::Exception("Error in my::http::Client::send.",
                                  WSAGetLastError());
    }
#else
    if (bytes < 0) {
      throw my::common::Exception("Error in my::http::Client::send.", errno);
    }
#endif

    if (bytes == 0) {
      break;
    }

    sent += bytes;
  } while (sent < size);
}

my::http::Request my::http::Client::receive() const {
  int bufsize = 1024;
  char *buf = new char[bufsize]();
  int received = 0;
  std::string end_of_read = "\r\n\r\n";
  int num_read = 1;
  while (true) {
    int bytes = ::recv(client->conn.get_socket(), buf + received, num_read, 0);

#ifdef _WIN32
    if (bytes == SOCKET_ERROR) {
      throw my::common::Exception("Error in my::http::Client::receive.",
                                  WSAGetLastError());
    }
#else
    if (bytes < 0) {
      throw my::common::Exception("Error in my::http::Client::receive.", errno);
    }
#endif

    received += num_read;

    if (bytes == 0) {
      break;
    }

    char *end_pos = strstr(buf, end_of_read.c_str());
    if (end_pos != NULL) {
      break;
    }

    if (received == bufsize) {
      int new_bufsize = bufsize * 2;
      char *new_buf = new char[new_bufsize]();
      strcpy(new_buf, buf);
      delete[] buf;
      bufsize = new_bufsize;
      buf = new_buf;
    }
  }

  auto req = Request::parse(buf);

  delete[] buf;

#ifdef _WIN32
  u_long len = 0;
  ioctlsocket(client->conn.get_socket(), FIONREAD, &len);
#else
  int len = 0;
  ioctl(client->conn.get_socket(), FIONREAD, &len);
#endif

  if (len == 0) {
    return req;
  }

  bufsize = 1024;
  buf = new char[bufsize]();
  received = 0;
  while (true) {
    int bytes =
        ::recv(client->conn.get_socket(), buf + received, len - received, 0);

#ifdef _WIN32
    if (bytes == SOCKET_ERROR) {
      throw my::common::Exception("Error in my::http::Client::receive.",
                                  WSAGetLastError());
    }
#else
    if (bytes < 0) {
      throw my::common::Exception("Error in my::http::Client::receive.", errno);
    }
#endif

    received += bytes;

    if (bytes == 0 || received == len) {
      break;
    }

    if (received == bufsize) {
      int new_bufsize = bufsize * 2;
      char *new_buf = new char[new_bufsize]();
      strcpy(new_buf, buf);
      delete[] buf;
      bufsize = new_bufsize;
      buf = new_buf;
    }
  }

  req.set_body(buf, received + 1);

  return req;
}

class Default404 : public my::http::AHandler {
public:
  void operator()(const my::http::Client &client,
                  const my::http::Request &request) {
    std::string answer = "HTTP/1.1 404 Not Found\r\n"
                         "Content-Type: text/plain\r\n"
                         "Content-Lenght: 13\r\n"
                         "\r\n";
    std::string body = "404 Not Found";

    auto response = my::http::Response::parse(answer);
    response.set_body(body.c_str(), body.size());

    client.send(response);
  }
};

class Default405 : public my::http::AHandler {
public:
  void operator()(const my::http::Client &client,
                  const my::http::Request &request) {
    std::string answer = "HTTP/1.1 405 Not Found\r\n"
                         "Content-Type: text/plain\r\n"
                         "Content-Lenght: 23\r\n"
                         "\r\n";

    std::string body = "405 Method Not Allowed";

    auto response = my::http::Response::parse(answer);
    response.set_body(body.c_str(), body.size());

    client.send(response);
  }
};

// Server
class my::http::Server::ServerImpl {
public:
  Connection conn;
  std::map<std::string, Configuration> url_to_config;
};

my::http::Server::Server(const Adress &addr) {
  server = std::make_shared<ServerImpl>();

  int res = ::bind(server->conn.get_socket(), addr.get_addr()->ai_addr,
                   addr.get_addr()->ai_addrlen);
#ifdef _WIN32
  if (res == SOCKET_ERROR) {
    throw my::common::Exception("Error in my::http::Server::Server.",
                                WSAGetLastError());
  }
#else
  if (res) {
    throw my::common::Exception("Error in my::http::Server::Server.", errno);
  }
#endif

  res = ::listen(server->conn.get_socket(), 5);
#ifdef _WIN32
  if (res == SOCKET_ERROR) {
    throw my::common::Exception("Error in my::http::Server::handle.",
                                WSAGetLastError());
  }
#else
  if (res) {
    throw my::common::Exception("Error in my::http::Server::handle.", errno);
  }
#endif
}

void my::http::Server::add_configuration(const Configuration &config) {
  server->url_to_config[config.url] = config;
}

void my::http::Server::handle() const {
  while (true) {
    Connection::socket_t sock =
        ::accept(server->conn.get_socket(), nullptr, nullptr);
#ifdef _WIN32
    if (sock == INVALID_SOCKET) {
      throw my::common::Exception("Error in my::http::Server::handle.",
                                  WSAGetLastError());
    }
#else
    if (sock == -1) {
      throw my::common::Exception("Error in my::http::Server::handle.", errno);
    }
#endif

    Client client(Connection{sock});
    auto request = client.receive();

    std::string url = request.get_url();
    std::string method = request.get_method();

    Configuration config;
    try {
      config = server->url_to_config.at(url);
    } catch (std::out_of_range) {
      Default404()(client, request);
      continue;
    }

    std::shared_ptr<AHandler> handler;
    try {
      handler = config.method_to_handler.at(method);
    } catch (std::out_of_range) {
      Default405()(client, request);
      continue;
    }

    (*handler)(client, request);
  }
}
