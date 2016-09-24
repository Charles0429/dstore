#include "inet_addr.h"
#include <netdb.h>
#include <cstring>
#include "errno_define.h"
#include "log.h"

using namespace dstore::common;
using namespace dstore::network;

InetAddr::InetAddr(void)
  : host_(nullptr), port_(nullptr), addr_(), addr_len_(sizeof(addr_)),
  ai_family_(-1), ai_socktype_(-1), ai_protocol_(-1), is_ipv6_(false),
  is_ai_passive_(false), is_resolved_(false)
{
}

InetAddr::~InetAddr(void)
{
}

int InetAddr::set_addr(const char *host, const char *port, const bool is_ipv6, const bool is_ai_passive)
{
  int ret = DSTORE_SUCCESS;
  host_ = host;
  port_ = port;
  is_ipv6_ = is_ipv6;
  is_ai_passive_ = is_ai_passive;
  is_resolved_ = false;
  if (DSTORE_SUCCESS != (ret = resolve())) {
    LOG_WARN("resolve address failed, host=%s, port=%s, is_ipv6=%d, is_ai_passive=%d",
        host, port, is_ipv6, is_ai_passive);
    return ret;
  }
  return ret;
}

const struct sockaddr* InetAddr::get_addr(void) const
{
  return &addr_;
}

struct sockaddr* InetAddr::get_addr(void)
{
  return &addr_;
}

const int *InetAddr::get_addr_len(void) const
{
  return &addr_len_;
}

int *InetAddr::get_addr_len(void)
{
  return &addr_len_;
}

int InetAddr::resolve(void)
{
  int ret = DSTORE_SUCCESS;
  struct addrinfo hints;
  struct addrinfo *res = nullptr;
  struct addrinfo *res_save = nullptr;

  ::memset(&hints, 0, sizeof(struct addrinfo));
  if (is_ai_passive_) {
    hints.ai_flags = AI_PASSIVE;
  }
  if (is_ipv6_) {
    hints.ai_family = AF_INET6;
  } else {
    hints.ai_family = AF_INET;
  }
  hints.ai_socktype = SOCK_STREAM;

  ret = ::getaddrinfo(host_, port_, &hints, &res);
  if (0 != ret) {
    LOG_WARN("getaddrinfo failed, host=%s, port=%s, ret=%d, error=%s", host_, port_, ret, gai_strerror(ret));
    ret = DSTORE_PARSE_ADDR_ERROR;
    return ret;
  } else if (nullptr == res) {
    LOG_WARN("getaddrinfo failed, got an empty address, host=%s, port=%s", host_, port_);
    ret = DSTORE_PARSE_ADDR_EMPTY;
    return ret;
  }
  addr_ = *res->ai_addr;
  addr_len_ = res->ai_addrlen;
  ai_family_ = res->ai_family;
  ai_socktype_ = res->ai_socktype;
  ai_protocol_ = res->ai_protocol;
  ::freeaddrinfo(res_save);
  is_resolved_ = true;
  return ret;
}
