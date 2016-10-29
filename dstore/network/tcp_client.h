#ifndef DSTORE_NETWORK_TCP_CLIENT_H_
#define DSTORE_NETWORK_TCP_CLIENT_H_

#include <unordered_map>
#include <functional>
#include <memory>
#include "event_loop.h"
#include "connection.h"
#include "tcp_connector.h"

namespace dstore
{
namespace network
{
class TCPClient
{
 public:
  typedef std::function<int(std::shared_ptr<Connection>)> MessageDecodeCallback;
  typedef std::function<int(std::shared_ptr<Connection>)> NewMessageCallback;

  TCPClient(void);
  ~TCPClient(void);
  int connect(const char *host, const char *port, const bool is_ipv6);
  void set_message_decode_callback(const MessageDecodeCallback &message_decode);
  void set_new_message_callback(const NewMessageCallback &new_message);
 private:
  int on_read(int fd, int type, void *args);
  int on_write(int fd, int type, void *args);
  void remove_connection(const int64_t connection_id);
  int on_close(const int64_t connection_id);
  int set_connection_status(const int64_t connection_id, const Connection::Status status);
 private:
  EventLoop loop_;
  TCPConnector connector_;
  std::unordered_map<int64_t, std::shared_ptr<Connection>> connection_map_;
  int64_t next_connection_id_;
  MessageDecodeCallback message_decoder_;
  NewMessageCallback new_message_callback_;
};
}  // namespace network
}  // namespace dstore

#endif  // DSTORE_NETWORK_TCP_CLIENT_H_
