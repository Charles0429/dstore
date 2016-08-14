#ifndef DSTORE_NETWORK_TCP_SERVER_H_
#define DSTORE_NETWORK_TCP_SERVER_H_

#include <unordered_map>
#include <string>
#include <functional>
#include "tcp_listener.h"
#include "event_loop.h"

namespace dstore
{
namespace network
{
class TCPListener;
class TCPServer;
class EventLoop;
class Connection;
class TCPServer
{
 public:
  typedef std::function<int(Connection *, bool &)> MessageDecodeCallback;
  typedef std::function<int(Connection *)> NewMessageCallback;
  TCPServer(void);
  TCPServer(const char *host, const char *port, const bool is_ipv6);
  ~TCPServer(void);
  int start(void);
  void loop(void);
  TCPServer &operator=(const TCPServer &) = delete;
  TCPServer(const TCPServer &) = delete;
 private:
  int on_connect(int fd, InetAddr *addr);
  int on_read(int fd, int type, void *args);
  int on_write(int fd, int type, void *args);
 private:
  std::string host_;
  std::string port_;
  bool is_ipv6_;
  EventLoop loop_;
  TCPListener listener_;
  std::unordered_map<int64_t, Connection *> connection_map_;
  int64_t next_connection_id_;
  MessageDecodeCallback message_decoder_;
  NewMessageCallback new_message_callback_;
};
}  // end namespace network
}  // end namespace dstore
#endif  // DSTORE_NETWORK_TCP_SERVER_H_
