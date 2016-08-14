#ifndef DSTORE_NETWORK_INET_ADDR_H_
#define DSTORE_NETWORK_INET_ADDR_H_

#include <arpa/inet.h>

namespace dstore
{
namespace network
{
class InetAddr
{
 public:
  InetAddr(void);
  ~InetAddr(void);
  int set_addr(const char *host, const char *port, const bool is_ipv6, const bool is_ai_passive);
  const struct sockaddr *get_addr(void) const;
  struct sockaddr *get_addr(void);
  const int *get_addr_len(void) const;
  int *get_addr_len(void);
  int get_family(void) const { return ai_family_; }
  int get_socket_type(void) const { return ai_socktype_; }
  int get_protocol(void) const { return ai_protocol_; }
 private:
  int resolve();
 private:
  const char *host_;
  const char *port_;
  struct sockaddr addr_;
  int addr_len_;
  int ai_family_;
  int ai_socktype_;
  int ai_protocol_;
  bool is_ipv6_;
  bool is_ai_passive_;
  bool is_resolved_;
};
}  // end namespace network
}  // end namespace dstore
#endif  // DSTORE_NETWORK_INET_ADDR_H_
