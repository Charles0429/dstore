#ifndef DSTORE_NETWORK_TCP_LISTENER_H_
#define DSTORE_NETWORK_TCP_LISTENER_H_

#include <functional>
#include "socket.h"
#include "event_loop.h"
#include "inet_addr.h"

namespace dstore
{
namespace network
{
class TCPListener
{
 public:
  typedef std::function<int(int fd, InetAddr *)> NewConnectionCallback;
  TCPListener(void);
  ~TCPListener(void);
  int set_addr(const char *host, const char *port, const bool is_ipv6);
  void set_new_connection_callback(NewConnectionCallback on_connect);
  Event &get_listen_event(void);
  int start(void);
  int accept(int fd, int type, void *args);
  TCPListener &operator=(const TCPListener &) = delete;
  TCPListener(const TCPListener &) = delete;
 private:
  InetAddr addr_;
  ListenSocket socket_;
  Event e_;
  NewConnectionCallback on_connect_;
};
}  // end namespace network
}  // end namespace dstore
#endif  // DSTORE_NETWORK_TCP_LISTENER_H_
