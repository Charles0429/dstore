#ifndef DSTORE_NETWORK_TCP_CONNECTOR_H_
#define DSTORE_NETWORK_TCP_CONNECTOR_H_

#include "inet_addr.h"
#include "socket.h"
#include "event_loop.h"

namespace dstore
{
namespace network
{
class TCPConnector
{
 public:
  TCPConnector(void);
  int set_addr(const char *host, const char *port, const bool is_ipv6);
  Event &get_event(void);
  int start(void);
  int connect(void);
  int get_socket_fd(void);
  InetAddr &get_server_addr(void);
  TCPConnector &operator=(const TCPConnector &) = delete;
  TCPConnector(const TCPConnector &) = delete;
 private:
  InetAddr addr_;
  Socket socket_;
};
}  // namespace network
}  // namespace dstore
#endif  // DSTORE_NETWORK_TCP_CONNECTOR_H_
