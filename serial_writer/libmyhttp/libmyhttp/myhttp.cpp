#include "myhttp.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <vector>

// my::Adress
class my::Adress::AdressImpl {
public:
  adress_t type;
  struct sockaddr_in addr;
};

my::Adress::Adress(const adress_t type, const struct sockaddr_in &addr) {
  adress = new AdressImpl;
  adress->addr = addr;
  adress->type = type;
}

my::Adress::~Adress() { delete adress; }

const struct sockaddr_in &my::Adress::get_addr() const { return adress->addr; }

const my::Adress::adress_t my::Adress::get_type() const { return adress->type; }

my::Adress my::Adress::get_by_name(const std::string &name, int port) {
  struct hostent *host = gethostbyname(name.c_str());

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  memcpy(&addr.sin_addr.s_addr, host->h_addr_list, host->h_length);

  return {adress_t::HOSTNAME, addr};
}

my::Adress my::Adress::get_by_ip(const std::string &ip, int port) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip.c_str());

  return {adress_t::IP, addr};
}

// my::Header
class my::Header::HeaderImpl {
public:
  header_t type;
  std::string key;
  std::string value;

  static const std::vector<std::pair<header_t, std::string>> TYPE_TO_KEY;
};

const std::vector<std::pair<my::Header::header_t, std::string>>
    my::Header::HeaderImpl::TYPE_TO_KEY = {
        {header_t::Host, "Host"},
        {header_t::Date, "Date"},
        {header_t::Content_type, "Content-type"},
        {header_t::Content_lenght, "Content-leght"},
        {header_t::Connection, "Connection"}};

my::Header::Header(const header_t type, const std::string &value,
                   const std::string &key) {
  header = new HeaderImpl;

  header->type = type;
  header->key = get_key_string(header->type, key);
  header->value = value;
}

my::Header::~Header() {
  delete header;
}

const std::string my::Header::get_str() const {
  std::stringstream ss;
  ss << header->key << ": " << header->value << "\r\n";
  return ss.str();
}

std::ofstream &operator<<(std::ofstream &out, const my::Header &obj) {
  out << obj.get_str();
  return out;
}

const std::string my::Header::get_key_string(header_t type,
                                             const std::string &fail_str) {
  auto it = std::find_if(HeaderImpl::TYPE_TO_KEY.begin(),
                         HeaderImpl::TYPE_TO_KEY.end(),
                         [type](const auto &p) { return p.first == type; });

  if (it == HeaderImpl::TYPE_TO_KEY.end()) {
    return fail_str;
  }

  return it->second;
}

const my::Header::header_t my::Header::get_type_name(const std::string &key,
                                                     const header_t fail_type) {
  auto it = std::find_if(HeaderImpl::TYPE_TO_KEY.begin(),
                         HeaderImpl::TYPE_TO_KEY.end(),
                         [key](const auto &p) { return p.first == key; });

  if (it == HeaderImpl::TYPE_TO_KEY.end()) {
    return fail_type;
  }

  return it->first;
}
