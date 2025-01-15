#include "myhttp.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <set>
#include <sstream>
#include <sys/socket.h>
#include <vector>

// my::http::Adress
class my::http::Adress::AdressImpl {
public:
  int port;
  std::string hostname;
  struct sockaddr_in addr;
};

my::http::Adress::Adress() {
  adress = std::make_shared<AdressImpl>();

  adress->port = 0;
  adress->hostname = "";
  memset(&adress->addr, 0, sizeof(adress->addr));
}

my::http::Adress::Adress(const std::string &name, int port) {
  adress = std::make_shared<AdressImpl>();

  adress->port = port;

  std::stringstream ss;
  ss << name;
  if (port != 80 && port != 443) {
    ss << ':' << adress->port;
    adress->hostname = ss.str();
  }

  adress->hostname = ss.str();

  struct hostent *host = gethostbyname(name.c_str());

  memset(&adress->addr, 0, sizeof(adress->addr));

  adress->addr.sin_family = AF_INET;
  adress->addr.sin_port = htons(port);
  memcpy(&adress->addr.sin_addr.s_addr, host->h_addr_list, host->h_length);
}

const struct sockaddr_in &my::http::Adress::get_addr() const {
  return adress->addr;
}

const int my::http::Adress::get_port() const { return adress->port; }

const std::string &my::http::Adress::get_hostname() const {
  return adress->hostname;
}

// my::http::Header
class my::http::Header::HeaderImpl {
public:
  header_t type;
  std::string key;
  std::string value;

  static const std::vector<std::pair<header_t, std::string>> TYPE_TO_KEY;
};

const std::vector<std::pair<my::http::Header::header_t, std::string>>
    my::http::Header::HeaderImpl::TYPE_TO_KEY = {
        {header_t::Host, "Host"},
        {header_t::Date, "Date"},
        {header_t::Content_type, "Content-type"},
        {header_t::Content_lenght, "Content-lenght"},
        {header_t::Connection, "Connection"}};

my::http::Header::Header(const header_t type, const std::string &value,
                         const std::string &key) {
  header = std::make_shared<HeaderImpl>();

  header->type = type;
  header->key = get_type_string(header->type, key);
  header->value = value;
}

my::http::Header::Header(const std::string &key, const std::string &value) {
  *this = Header(get_type_type(key), value);
}

const std::string my::http::Header::get_str() const {
  std::stringstream ss;
  ss << header->key << ": " << header->value << "\r\n";
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

const std::string
my::http::Header::get_type_string(header_t type, const std::string &fail_str) {
  auto it = std::find_if(HeaderImpl::TYPE_TO_KEY.begin(),
                         HeaderImpl::TYPE_TO_KEY.end(),
                         [type](const auto &p) { return p.first == type; });

  if (it == HeaderImpl::TYPE_TO_KEY.end()) {
    return fail_str;
  }

  return it->second;
}

const my::http::Header::header_t
my::http::Header::get_type_type(const std::string &name,
                                const header_t fail_type) {
  auto it = std::find_if(HeaderImpl::TYPE_TO_KEY.begin(),
                         HeaderImpl::TYPE_TO_KEY.end(),
                         [name](const auto &p) { return p.second == name; });

  if (it == HeaderImpl::TYPE_TO_KEY.end()) {
    return fail_type;
  }

  return it->first;
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

const std::string my::http::Param::get_str() const {
  std::stringstream ss;
  ss << param->key << '=' << param->value;
  return ss.str();
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

  request_t type;
  std::string url;
  const std::string HTTP_VER = "HTTP/1.1";
  std::set<Header> headers;
  char *body = nullptr;
  int body_size = 0;

  static const std::vector<std::pair<request_t, std::string>> TYPE_TO_KEY;
};

const std::vector<std::pair<my::http::Request::request_t, std::string>>
    my::http::Request::RequestImpl::TYPE_TO_KEY = {{request_t::GET, "GET"},
                                                   {request_t::POST, "POST"}};

void my::http::Request::set_adress(const my::http::Adress &addr) {
  *request->addr = addr;
}

const my::http::Adress &my::http::Request::get_adress() const {
  return *request->addr;
}

void my::http::Request::set_type(const my::http::Request::request_t type) {
  request->type = type;
}

const my::http::Request::request_t my::http::Request::get_type() const {
  return request->type;
}

void my::http::Request::set_url(const std::string &url) { request->url = url; }

const std::string &my::http::Request::get_url() const { return request->url; }

void my::http::Request::add_header(const Header &header) {
  request->headers.insert(header);
}

const std::set<my::http::Header> &my::http::Request::get_headers() const {
  return request->headers;
}

void my::http::Request::set_body(const char *body, const int body_lenght) {
  request->body = new char[body_lenght];
  memcpy(request->body, body, body_lenght);
  request->body_size = body_lenght;
}

const std::pair<const char *, int> my::http::Request::get_body() const {
  return {request->body, request->body_size};
}

const std::pair<std::unique_ptr<char[]>, int> my::http::Request::dump() {
  std::stringstream ss;

  ss << get_type_string(request->type) << ' ' << request->url << ' '
     << request->HTTP_VER << "\r\n";

  for (auto &it : request->headers) {
    ss << it << "\r\n";
  }

  ss << "\r\n";

  std::string msg = ss.str();

  size_t size = msg.size() + request->body_size;
  std::unique_ptr<char[]> ptr(new char[size]);

  memcpy(ptr.get(), msg.c_str(), msg.size()); 
  memcpy(ptr.get() + msg.size() + 1, request->body, request->body_size);

  return {std::move(ptr), size};
}

std::vector<std::string> split(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

#include <iostream>
my::http::Request my::http::Request::parse(const std::string &request) {
  auto lines = split(request, "\r\n");

  for (auto &it : lines) {
    std::cout << it << '\n';
  }

  Request req;

  

  return {};
}

const std::string my::http::Request::get_type_string(request_t type) {
  auto it = std::find_if(RequestImpl::TYPE_TO_KEY.begin(),
                         RequestImpl::TYPE_TO_KEY.end(),
                         [type](const auto &p) { return p.first == type; });

  return it->second;
}

const my::http::Request::get_type my::http::Request::get_type_type(std::string type) {
  auto it = std::find_if(RequestImpl::TYPE_TO_KEY.begin(),
                         RequestImpl::TYPE_TO_KEY.end(),
                         [type](const auto &p) { return p.first == type; });

  return it->second;

// my::Response
// class my::http::Response::ResponseImpl {
// public:

//   step_t cur_step;
// };

// my::http::Response::Response() {
//   response = std::make_shared<ResponseImpl>();
// }

// my::http::Response::Response(const int status, const std::string &phrase,
//            std::vector<Header> headers, const char *body,
//            const int body_lenght) {

// }
