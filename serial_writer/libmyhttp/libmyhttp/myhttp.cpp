#include "myhttp.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

// my::Adress
class my::Adress::AdressImpl {
public:
  struct sockaddr_in addr;
};

my::Adress::Adress(const struct sockaddr_in &addr) {
  adress = new AdressImpl;
  adress->addr = addr;
}

my::Adress::~Adress() { delete adress; }

my::Adress my::Adress::get_by_name(const std::string &name, int port) {
  struct hostent *host = gethostbyname(name.c_str());

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  memcpy(&addr.sin_addr.s_addr, host->h_addr_list, host->h_length);

  return {addr};
}

my::Adress my::Adress::get_by_ip(const std::string &ip, int port) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip.c_str());

  return {addr};
}

const struct sockaddr_in &my::Adress::get_addr() const {
  return adress->addr;
}
