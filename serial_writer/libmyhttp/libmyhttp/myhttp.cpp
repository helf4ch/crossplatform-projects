#include "myhttp.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
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

const std::string my::http::Header::get_str() const {
  std::stringstream ss;
  ss << header->key << ": " << header->value << "\r\n";
  return ss.str();
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
  std::string request_str;

  static const std::vector<std::pair<request_t, std::string>> TYPE_TO_KEY;
};

const std::vector<std::pair<my::http::Request::request_t, std::string>>
    my::http::Request::RequestImpl::TYPE_TO_KEY = {{request_t::GET, "GET"},
                                                   {request_t::POST, "POST"}};

my::http::Request::Request(const Adress &addr, const std::string &request_str) {
  request = std::make_shared<RequestImpl>();

  request->addr = std::make_unique<Adress>(addr);
  request->request_str = request_str;
}

const my::http::Adress &my::http::Request::get_adress() const {
  return *request->addr;
}

const std::string my::http::Request::get_str() const {
  return request->request_str;
}

const std::string my::http::Request::get_type_string(request_t type) {
  auto it = std::find_if(RequestImpl::TYPE_TO_KEY.begin(),
                         RequestImpl::TYPE_TO_KEY.end(),
                         [type](const auto &p) { return p.first == type; });

  return it->second;
}

my::http::Request
my::http::Request::gen(const request_t type, const Adress &addr,
                       const std::string &path, std::vector<Header> headers,
                       const std::vector<Param> &params, const char *body,
                       const int body_lenght) {
  std::stringstream ss;

  ss << get_type_string(type) << ' ' << path << '?';

  for (auto it = params.begin(); it < params.end(); ++it) {
    ss << *it;
    if (it < params.end() - 1) {
      ss << '&';
    }
  }

  ss << " HTML/1.1\r\n";

  headers.insert(headers.begin(),
                 Header(Header::header_t::Host, addr.get_hostname()));

  for (auto &it : headers) {
    ss << it;
  }

  ss << "\r\n";

  if (body) {
    ss.write(body, body_lenght);
  }

  return {addr, ss.str()};
}
