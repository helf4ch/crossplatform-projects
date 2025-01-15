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

  std::string type;
  std::string url;
  std::string http_ver = "HTTP/1.1";
  std::set<Param> params;
  std::set<Header> headers;
  char *body = nullptr;
  int body_size = 0;
};

my::http::Request::Request() { request = std::make_shared<RequestImpl>(); }

void my::http::Request::set_adress(const my::http::Adress &addr) {
  *request->addr = addr;
}

const my::http::Adress &my::http::Request::get_adress() const {
  return *request->addr;
}

void my::http::Request::set_type(const std::string &type) {
  request->type = type;
}

const std::string my::http::Request::get_type() const { return request->type; }

void my::http::Request::set_url(const std::string &url) { request->url = url; }

void my::http::Request::add_param(const Param &param) {
  request->params.insert(param);
}

my::http::Param &my::http::Request::get_param(const std::string &key) {
  return const_cast<Param &>(
      *std::find_if(request->params.begin(), request->params.end(),
                    [key](auto &it) { return it.get_key() == key; }));
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

my::http::Header &my::http::Request::get_header(const std::string key) {
  return const_cast<Header &>(
      *std::find_if(request->headers.begin(), request->headers.end(),
                    [key](auto &it) { return it.get_key() == key; }));
}

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

  ss << request->type << ' ' << request->url << '?';

  for (auto &it : request->params) {
    ss << it << '?';
  }

  ss << ' ' << request->http_ver << "\r\n";

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
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back(token);
  }

  res.push_back(s.substr(pos_start));
  return res;
}

my::http::Request my::http::Request::parse(const std::string &request) {
  auto lines = split(request, "\r\n");

  if (lines.back() != "\r\n") {
    // error
  }

  Request req;

  std::stringstream ss(lines[0]);
  std::string buf;

  ss >> buf;
  req.set_type(buf);

  ss >> buf;
  auto url_split = split(buf, "?");
  req.set_url(url_split[0]);

  for (int i = 1; i < url_split.size(); ++i) {
    auto pos = url_split[i].find("=");
    std::string key = url_split[i].substr(0, pos);
    std::string value = url_split[i].substr(pos + 1, url_split[i].size());
    req.add_param({key, value});
  }

  ss >> buf;
  req.set_http_ver(buf);

  for (int i = 1; i < lines.size() - 2; ++i) {
    auto pos = lines[i].find(":");
    std::string key = lines[i].substr(0, pos);
    std::stringstream ss(lines[i].substr(pos + 1, lines[i].size()));
    std::string value;
    ss >> value;
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
  char *body = nullptr;
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

my::http::Header &my::http::Response::get_header(const std::string key) {
  return const_cast<Header &>(
      *std::find_if(response->headers.begin(), response->headers.end(),
                    [key](auto &it) { return it.get_key() == key; }));
}

const std::set<my::http::Header> &my::http::Response::get_headers() const {
  return response->headers;
}

void my::http::Response::set_body(const char *body, const int body_lenght) {
  response->body = new char[body_lenght];
  memcpy(response->body, body, body_lenght);
  response->body_size = body_lenght;
}

const std::pair<const char *, int> my::http::Response::get_body() const {
  return {response->body, response->body_size};
}

const std::pair<std::unique_ptr<char[]>, int> my::http::Response::dump() {
  std::stringstream ss;

  ss << response->http_ver << ' ' << response->code << ' ' << response->text
     << "\r\n";

  for (auto &it : response->headers) {
    ss << it << "\r\n";
  }

  ss << "\r\n";

  std::string msg = ss.str();

  size_t size = msg.size() + response->body_size;
  std::unique_ptr<char[]> ptr(new char[size]);

  memcpy(ptr.get(), msg.c_str(), msg.size());
  memcpy(ptr.get() + msg.size() + 1, response->body, response->body_size);

  return {std::move(ptr), size};
}

my::http::Response my::http::Response::parse(const std::string &request) {
  auto lines = split(request, "\r\n");

  if (lines.back() != "\r\n") {
    // error
  }

  Response res;

  std::stringstream ss(lines[0]);
  std::string buf;

  ss >> buf;
  res.set_http_ver(buf);

  ss >> buf;
  res.set_code(std::stoi(buf));

  ss >> buf;
  res.set_text(buf);

  for (int i = 1; i < lines.size() - 1; ++i) {
    auto pos = lines[i].find(":");
    std::string key = lines[i].substr(0, pos);
    std::stringstream ss(lines[i].substr(pos + 1, lines[i].size()));
    std::string value;
    ss >> value;
    res.add_header({key, value});
  }

  return res;
}
